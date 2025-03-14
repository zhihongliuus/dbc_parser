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
    // Create a database manually instead of parsing from DBC content
    db_ = std::make_unique<Database>();
    
    // Set version
    Database::Version version;
    version.version = "1.0";
    db_->set_version(version);
    
    // Set bit timing
    Database::BitTiming bit_timing;
    bit_timing.baudrate = 500000;
    bit_timing.btr1 = 1;
    bit_timing.btr2 = 10;
    db_->set_bit_timing(bit_timing);
    
    // Add nodes
    auto ecu1 = std::make_unique<Node>("ECU1");
    auto ecu2 = std::make_unique<Node>("ECU2");
    auto ecu3 = std::make_unique<Node>("ECU3");
    db_->add_node(std::move(ecu1));
    db_->add_node(std::move(ecu2));
    db_->add_node(std::move(ecu3));
    
    // Add EngineData message
    auto engine_msg = std::make_unique<Message>(100, "EngineData", 8, "ECU1");
    
    // Add signals to EngineData
    auto engine_speed = std::make_unique<Signal>(
      "EngineSpeed", 0, 16, true, false, 0.1, 0.0, 0.0, 6500.0, "rpm"
    );
    engine_speed->add_receiver("ECU2");
    engine_speed->add_receiver("ECU3");
    engine_msg->add_signal(std::move(engine_speed));
    
    auto engine_temp = std::make_unique<Signal>(
      "EngineTemp", 16, 8, true, false, 1.0, -40.0, -40.0, 215.0, "degC"
    );
    engine_temp->add_receiver("ECU2");
    engine_msg->add_signal(std::move(engine_temp));
    
    auto engine_load = std::make_unique<Signal>(
      "EngineLoad", 24, 8, true, false, 1.0, 0.0, 0.0, 100.0, "%"
    );
    engine_load->add_receiver("ECU2");
    engine_load->add_receiver("ECU3");
    engine_msg->add_signal(std::move(engine_load));
    
    db_->add_message(std::move(engine_msg));
    
    // Add TransmissionData message
    auto trans_msg = std::make_unique<Message>(200, "TransmissionData", 6, "ECU2");
    
    // Add signals to TransmissionData
    auto gear_pos = std::make_unique<Signal>(
      "GearPosition", 0, 4, true, false, 1.0, 0.0, 0.0, 8.0, ""
    );
    gear_pos->add_receiver("ECU1");
    gear_pos->add_receiver("ECU3");
    
    // Add value descriptions for GearPosition
    gear_pos->add_value_description(0, "Neutral");
    gear_pos->add_value_description(1, "First");
    gear_pos->add_value_description(2, "Second");
    gear_pos->add_value_description(3, "Third");
    gear_pos->add_value_description(4, "Fourth");
    gear_pos->add_value_description(5, "Fifth");
    gear_pos->add_value_description(6, "Sixth");
    gear_pos->add_value_description(7, "Reverse");
    gear_pos->add_value_description(8, "Park");
    
    trans_msg->add_signal(std::move(gear_pos));
    
    auto trans_temp = std::make_unique<Signal>(
      "TransmissionTemp", 8, 8, true, false, 1.0, -40.0, -40.0, 215.0, "degC"
    );
    trans_temp->add_receiver("ECU1");
    trans_msg->add_signal(std::move(trans_temp));
    
    auto trans_speed = std::make_unique<Signal>(
      "TransmissionSpeed", 16, 16, true, false, 0.1, 0.0, 0.0, 6500.0, "rpm"
    );
    trans_speed->add_receiver("ECU1");
    trans_speed->add_receiver("ECU3");
    trans_msg->add_signal(std::move(trans_speed));
    
    db_->add_message(std::move(trans_msg));
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