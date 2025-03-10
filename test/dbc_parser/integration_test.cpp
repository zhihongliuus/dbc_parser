#include "dbc_parser/parser.h"
#include "dbc_parser/decoder.h"
#include "dbc_parser/types.h"

#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <vector>
#include <map>

namespace dbc_parser {
namespace testing {

class IntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    dbc_content_ = 
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
      " SG_ TransmissionMode M : 4|2@1+ (1,0) [0|3] \"\" ECU1,ECU3\n"
      " SG_ TransmissionTemp : 8|8@1+ (1,-40) [-40|215] \"degC\" ECU1\n"
      " SG_ TransmissionSpeed : 16|16@1+ (0.1,0) [0|6500] \"rpm\" ECU1,ECU3\n"
      " SG_ TransmissionInfo m0 : 32|8@1+ (1,0) [0|255] \"\" ECU1\n"
      " SG_ TransmissionPressure m1 : 32|8@1+ (1,0) [0|255] \"kPa\" ECU1\n"
      "\n"
      "CM_ BU_ ECU1 \"Engine Control Unit\";\n"
      "CM_ BU_ ECU2 \"Transmission Control Unit\";\n"
      "CM_ BU_ ECU3 \"Diagnostic Unit\";\n"
      "CM_ BO_ 100 \"Engine data message\";\n"
      "CM_ SG_ 100 EngineSpeed \"Engine speed in RPM\";\n"
      "CM_ SG_ 100 EngineTemp \"Engine temperature in degrees Celsius\";\n"
      "CM_ SG_ 100 EngineLoad \"Engine load as percentage\";\n"
      "CM_ BO_ 200 \"Transmission data message\";\n"
      "CM_ SG_ 200 GearPosition \"Current gear position\";\n"
      "\n"
      "BA_DEF_ SG_ \"SignalType\" STRING ;\n"
      "BA_DEF_ BO_ \"GenMsgCycleTime\" INT 0 10000;\n"
      "BA_DEF_DEF_ \"SignalType\" \"\";\n"
      "BA_DEF_DEF_ \"GenMsgCycleTime\" 100;\n"
      "BA_ \"GenMsgCycleTime\" BO_ 100 100;\n"
      "BA_ \"GenMsgCycleTime\" BO_ 200 200;\n"
      "\n"
      "VAL_ 200 GearPosition 0 \"Neutral\" 1 \"First\" 2 \"Second\" 3 \"Third\" 4 \"Fourth\" 5 \"Fifth\" 6 \"Sixth\" 7 \"Reverse\" 8 \"Park\";\n"
      "VAL_ 200 TransmissionMode 0 \"Normal\" 1 \"Sport\" 2 \"Economy\" 3 \"Winter\";\n";
  }
  
  void TearDown() override {
  }
  
  std::string dbc_content_;
};

TEST_F(IntegrationTest, CompleteWorkflow) {
  // 1. Parse DBC content
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, ParserOptions());
  ASSERT_TRUE(db);
  
  // 2. Create a decoder
  DecoderOptions decoder_options;
  Decoder decoder(std::make_shared<Database>(*db), decoder_options);
  
  // 3. Create some CAN frame data manually
  // Engine data: Speed=1000rpm, Temp=80°C, Load=50%
  std::vector<uint8_t> engine_data = {0xE8, 0x03, 0x78, 0x32};
  
  // 4. Decode the engine data frame
  auto engine_result = decoder.decode_frame(100, engine_data);
  ASSERT_TRUE(engine_result);
  EXPECT_EQ("EngineData", engine_result->name);
  
  // 5. Verify decoded values
  ASSERT_TRUE(engine_result->signals.find("EngineSpeed") != engine_result->signals.end());
  EXPECT_DOUBLE_EQ(100.0, engine_result->signals["EngineSpeed"].value);  // 0x03E8 = 1000, factor=0.1
  EXPECT_EQ("rpm", engine_result->signals["EngineSpeed"].unit);
  
  ASSERT_TRUE(engine_result->signals.find("EngineTemp") != engine_result->signals.end());
  EXPECT_DOUBLE_EQ(80.0, engine_result->signals["EngineTemp"].value);  // 0x78 = 120, offset=-40
  EXPECT_EQ("degC", engine_result->signals["EngineTemp"].unit);
  
  ASSERT_TRUE(engine_result->signals.find("EngineLoad") != engine_result->signals.end());
  EXPECT_DOUBLE_EQ(50.0, engine_result->signals["EngineLoad"].value);  // 0x32 = 50
  EXPECT_EQ("%", engine_result->signals["EngineLoad"].unit);
  
  // 6. Transmission data with mode 1 (Sport): Gear=3, Mode=1, Temp=60°C, Speed=1000rpm, Pressure=100kPa
  std::vector<uint8_t> trans_data = {0x13, 0x78, 0xE8, 0x03, 0x64};
  
  // 7. Decode the transmission data frame
  auto trans_result = decoder.decode_frame(200, trans_data);
  ASSERT_TRUE(trans_result);
  EXPECT_EQ("TransmissionData", trans_result->name);
  
  // 8. Verify the multiplexed signal decoding
  EXPECT_EQ(5, trans_result->signals.size());
  
  ASSERT_TRUE(trans_result->signals.find("GearPosition") != trans_result->signals.end());
  EXPECT_DOUBLE_EQ(3.0, trans_result->signals["GearPosition"].value);
  EXPECT_EQ("Third", trans_result->signals["GearPosition"].description);
  
  ASSERT_TRUE(trans_result->signals.find("TransmissionMode") != trans_result->signals.end());
  EXPECT_DOUBLE_EQ(1.0, trans_result->signals["TransmissionMode"].value);
  EXPECT_EQ("Sport", trans_result->signals["TransmissionMode"].description);
  
  ASSERT_TRUE(trans_result->signals.find("TransmissionTemp") != trans_result->signals.end());
  EXPECT_DOUBLE_EQ(80.0, trans_result->signals["TransmissionTemp"].value);
  
  ASSERT_TRUE(trans_result->signals.find("TransmissionSpeed") != trans_result->signals.end());
  EXPECT_DOUBLE_EQ(100.0, trans_result->signals["TransmissionSpeed"].value);
  
  // Verify that the correct multiplexed signal is present
  ASSERT_TRUE(trans_result->signals.find("TransmissionPressure") != trans_result->signals.end());
  EXPECT_DOUBLE_EQ(100.0, trans_result->signals["TransmissionPressure"].value);
  EXPECT_EQ("kPa", trans_result->signals["TransmissionPressure"].unit);
  
  // And the other multiplexed signal is not present
  EXPECT_TRUE(trans_result->signals.find("TransmissionInfo") == trans_result->signals.end());
  
  // 9. Test decoding specific signals directly
  auto gear_signal = decoder.decode_signal(200, "GearPosition", trans_data);
  ASSERT_TRUE(gear_signal);
  EXPECT_DOUBLE_EQ(3.0, gear_signal->value);
  EXPECT_EQ("Third", gear_signal->description);
  
  // 10. Test error handling for an unknown message ID
  auto unknown_result = decoder.decode_frame(999, {0x00, 0x00, 0x00, 0x00});
  ASSERT_TRUE(unknown_result);  // Should return a placeholder with default options
  EXPECT_EQ("UNKNOWN_999", unknown_result->name);
  EXPECT_TRUE(unknown_result->signals.empty());
  
  // 11. Test getting a value description directly
  auto desc = decoder.get_value_description(200, "GearPosition", 0);
  ASSERT_TRUE(desc);
  EXPECT_EQ("Neutral", *desc);
  
  // 12. Test database modification
  auto new_db = std::make_unique<Database>();
  
  // Add a message
  auto new_msg = std::make_unique<Message>(300, "NewMessage", 8, "ECU1");
  auto msg_ptr = new_db->add_message(std::move(new_msg));
  ASSERT_NE(nullptr, msg_ptr);
  
  // Add a signal
  auto new_signal = std::make_unique<Signal>("NewSignal", 0, 8, true, false, 1.0, 0.0, 0.0, 255.0, "");
  auto signal_ptr = msg_ptr->add_signal(std::move(new_signal));
  ASSERT_NE(nullptr, signal_ptr);
  
  // Create sample data and encode a value
  std::vector<uint8_t> new_data(8, 0);
  signal_ptr->encode(123.0, new_data);
  
  // Create a decoder for the new database
  Decoder new_decoder(std::make_shared<Database>(*new_db), decoder_options);
  
  // Decode the frame and verify
  auto new_result = new_decoder.decode_frame(300, new_data);
  ASSERT_TRUE(new_result);
  ASSERT_TRUE(new_result->signals.find("NewSignal") != new_result->signals.end());
  EXPECT_DOUBLE_EQ(123.0, new_result->signals["NewSignal"].value);
  
  // 13. Serialize the database to a string
  std::string output = parser->write_string(*db);
  EXPECT_FALSE(output.empty());
  
  // Verify the output contains expected elements
  EXPECT_TRUE(output.find("VERSION") != std::string::npos);
  EXPECT_TRUE(output.find("EngineData") != std::string::npos);
  EXPECT_TRUE(output.find("GearPosition") != std::string::npos);
  
  // 14. Parse the serialized output
  auto reparsed_db = parser->parse_string(output, ParserOptions());
  ASSERT_TRUE(reparsed_db);
  
  // Verify the reparsed database has the same basic structure
  EXPECT_EQ(db->messages().size(), reparsed_db->messages().size());
  
  // Test message still exists
  auto reparsed_msg = reparsed_db->get_message(100);
  ASSERT_NE(nullptr, reparsed_msg);
  EXPECT_EQ("EngineData", reparsed_msg->name());
  
  // 15. Test with the factory to create a parser based on file extension
  auto dbc_parser = ParserFactory::create_parser("test.dbc");
  ASSERT_NE(nullptr, dbc_parser);
  EXPECT_TRUE(dynamic_cast<DbcParser*>(dbc_parser.get()) != nullptr);
  
  // Test with unsupported extension
  EXPECT_THROW(ParserFactory::create_parser("test.unknown"), std::runtime_error);
}

} // namespace testing
} // namespace dbc_parser

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
} 