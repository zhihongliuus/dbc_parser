#include "dbc_parser/decoder.h"
#include "dbc_parser/parser.h"
#include "dbc_parser/types.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

namespace dbc_parser {
namespace testing {

class DecoderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    auto parser = std::make_unique<DbcParser>();
    ParserOptions options;
    options.verbose = true;
    
    std::string dbc_content = 
      "VERSION \"1.0\"\n"
      "\n"
      "NS_ :\n"
      "    NS_DESC_\n"
      "    CM_\n"
      "    BA_DEF_\n"
      "    BA_\n"
      "    VAL_\n"
      "    CAT_DEF_\n"
      "    CAT_\n"
      "    FILTER\n"
      "\n"
      "BS_: 500000,1,10\n"
      "\n"
      "BU_: ECU1 ECU2 ECU3\n"
      "\n"
      "BO_ 100 EngineData: 8 ECU1\n"
      " SG_ EngineSpeed : 0|16@1+ (0.1,0) [0|6500] \"rpm\" ECU2,ECU3\n"
      " SG_ EngineTemp : 16|8@1+ (1,-40) [-40|215] \"degC\" ECU2\n"
      " SG_ EngineLoad : 24|8@1+ (1,0) [0|100] \"%\" ECU2,ECU3\n"
      "\n"
      "BO_ 200 TransmissionData: 6 ECU2\n"
      " SG_ GearPosition : 0|4@1+ (1,0) [0|8] \"\" ECU1,ECU3\n"
      " SG_ TransmissionTemp : 8|8@1+ (1,-40) [-40|215] \"degC\" ECU1\n"
      " SG_ TransmissionSpeed : 16|16@1+ (0.1,0) [0|6500] \"rpm\" ECU1,ECU3\n"
      "\n"
      "VAL_ 200 GearPosition 0 \"Neutral\" 1 \"First\" 2 \"Second\" 3 \"Third\" 4 \"Fourth\" 5 \"Fifth\" 6 \"Sixth\" 7 \"Reverse\" 8 \"Park\";\n";
    
    db_ = parser->parse_string(dbc_content, options);
  }
  
  void TearDown() override {
  }
  
  std::unique_ptr<Database> db_;
};

TEST_F(DecoderTest, DecodeWithDefaultOptions) {
  // Create decoder with default options
  DecoderOptions opts;
  Decoder decoder(*db_, opts);
  
  // Test decoding engine data
  std::vector<uint8_t> engine_data = {0xE8, 0x03, 0x50, 0x40, 0x00, 0x00, 0x00, 0x00};  // 1000 rpm, 80C, 64%, padded to 8 bytes
  auto decoded = decoder.decode_frame(100, engine_data);
  ASSERT_TRUE(decoded);
  EXPECT_EQ("EngineData", decoded->name);
  EXPECT_EQ(3, decoded->signals.size());
  
  ASSERT_TRUE(decoded->signals.find("EngineSpeed") != decoded->signals.end());
  EXPECT_DOUBLE_EQ(100.0, decoded->signals.at("EngineSpeed").value);
  EXPECT_EQ("rpm", decoded->signals.at("EngineSpeed").unit);
  
  ASSERT_TRUE(decoded->signals.find("EngineTemp") != decoded->signals.end());
  EXPECT_DOUBLE_EQ(40.0, decoded->signals.at("EngineTemp").value);  // 80-40 (offset)
  EXPECT_EQ("degC", decoded->signals.at("EngineTemp").unit);
  
  ASSERT_TRUE(decoded->signals.find("EngineLoad") != decoded->signals.end());
  EXPECT_DOUBLE_EQ(64.0, decoded->signals.at("EngineLoad").value);
  EXPECT_EQ("%", decoded->signals.at("EngineLoad").unit);
}

TEST_F(DecoderTest, DecodeSpecificSignal) {
  // Create decoder
  DecoderOptions opts;
  Decoder decoder(*db_, opts);
  
  // Test decoding a specific signal
  std::vector<uint8_t> engine_data = {0xE8, 0x03, 0x50, 0x40, 0x00, 0x00, 0x00, 0x00};  // 1000 rpm, 80C, 64%, padded to 8 bytes
  auto result = decoder.decode_signal(100, "EngineSpeed", engine_data);
  ASSERT_TRUE(result);
  EXPECT_EQ("EngineSpeed", result->name);
  EXPECT_DOUBLE_EQ(100.0, result->value);
  EXPECT_EQ("rpm", result->unit);
  
  // Test non-existent signal
  auto not_found = decoder.decode_signal(100, "NonExistentSignal", engine_data);
  EXPECT_FALSE(not_found);
}

TEST_F(DecoderTest, GetValueDescription) {
  // Create decoder
  DecoderOptions opts;
  Decoder decoder(*db_, opts);
  
  // Test value descriptions
  auto trans_data = std::vector<uint8_t>{0x03, 0x00, 0x00, 0x00, 0x00, 0x00};  // Gear position 3
  auto desc = decoder.get_value_description(200, "GearPosition", 3);
  ASSERT_TRUE(desc);
  EXPECT_EQ("Third", *desc);
  
  // Test non-existent value
  auto not_found_val = decoder.get_value_description(200, "GearPosition", 99);
  EXPECT_FALSE(not_found_val);
  
  // Test non-existent signal
  auto not_found_sig = decoder.get_value_description(200, "NonExistentSignal", 0);
  EXPECT_FALSE(not_found_sig);
}

TEST_F(DecoderTest, UnknownMessageIdHandling) {
  // Default options (ignore_unknown_ids = true)
  DecoderOptions default_opts;
  EXPECT_TRUE(default_opts.ignore_unknown_ids);
  Decoder default_decoder(*db_, default_opts);
  
  // Test decoding with unknown message ID
  std::vector<uint8_t> data = {0x00, 0x00, 0x00, 0x00};
  auto default_result = default_decoder.decode_frame(999, data);
  ASSERT_TRUE(default_result);
  EXPECT_EQ(999u, default_result->id);
  EXPECT_EQ("UNKNOWN_999", default_result->name);
  EXPECT_TRUE(default_result->signals.empty());
  
  // Strict options (ignore_unknown_ids = false)
  DecoderOptions strict_opts;
  strict_opts.ignore_unknown_ids = false;
  Decoder strict_decoder(*db_, strict_opts);
  
  // Test decoding with unknown message ID in strict mode
  auto strict_result = strict_decoder.decode_frame(999, data);
  EXPECT_FALSE(strict_result);
}

TEST_F(DecoderTest, VerboseOption) {
  // Create decoder with verbose option
  DecoderOptions verbose_opts;
  verbose_opts.verbose = true;
  Decoder verbose_decoder(*db_, verbose_opts);
  
  // Just ensuring it doesn't crash when verbose is on
  std::vector<uint8_t> engine_data = {0xE8, 0x03, 0x50, 0x40, 0x00, 0x00, 0x00, 0x00};  // 1000 rpm, 80C, 64%, padded to 8 bytes
  auto result = verbose_decoder.decode_frame(100, engine_data);
  EXPECT_TRUE(result);
}

TEST_F(DecoderTest, DataTooShort) {
  // Create decoder
  DecoderOptions opts;
  Decoder decoder(*db_, opts);
  
  // Test with data that's too short
  std::vector<uint8_t> short_data = {0x00};
  // Using message ID 100 (EngineData) which expects 8 bytes
  auto result = decoder.decode_frame(100, short_data);
  EXPECT_FALSE(result);
}

} // namespace testing
} // namespace dbc_parser

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
} 