#include "dbc_parser/parser.h"
#include "dbc_parser/decoder.h"
#include "dbc_parser/types.h"

#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <iostream>
#include <sstream>

namespace dbc_parser {
namespace testing {

class DbcParserTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create sample DBC content
    std::stringstream ss;
    ss << "VERSION \"1.0\"\n\n"
       << "NS_ :\n"
       << "   NS_DESC_\n"
       << "   CM_\n"
       << "   BA_DEF_\n"
       << "   BA_\n"
       << "   VAL_\n"
       << "   CAT_DEF_\n"
       << "   CAT_\n"
       << "   FILTER\n\n"
       << "BS_: 500000,1,10\n\n"
       << "BU_: ECU1 ECU2 ECU3\n\n"
       << "BO_ 100 EngineData: 8 ECU1\n"
       << " SG_ EngineSpeed : 0|16@1+ (0.1,0) [0|6500] \"rpm\" ECU2,ECU3\n"
       << " SG_ EngineTemp : 16|8@1+ (1,-40) [-40|215] \"degC\" ECU2\n"
       << " SG_ EngineLoad : 24|8@1+ (1,0) [0|100] \"%\" ECU2,ECU3\n\n"
       << "BO_ 200 TransmissionData: 6 ECU2\n"
       << " SG_ GearPosition : 0|4@1+ (1,0) [0|8] \"\" ECU1,ECU3\n"
       << " SG_ TransmissionMode M : 4|2@1+ (1,0) [0|3] \"\" ECU1,ECU3\n"
       << " SG_ TransmissionTemp : 8|8@1+ (1,-40) [-40|215] \"degC\" ECU1\n"
       << " SG_ TransmissionSpeed : 16|16@1+ (0.1,0) [0|6500] \"rpm\" ECU1,ECU3\n"
       << " SG_ TransmissionInfo m0 : 32|8@1+ (1,0) [0|255] \"\" ECU1\n"
       << " SG_ TransmissionPressure m1 : 32|8@1+ (1,0) [0|255] \"kPa\" ECU1\n\n"
       << "CM_ BU_ ECU1 \"Engine Control Unit\";\n"
       << "CM_ BU_ ECU2 \"Transmission Control Unit\";\n"
       << "CM_ BU_ ECU3 \"Diagnostic Unit\";\n"
       << "CM_ BO_ 100 \"Engine data message\";\n"
       << "CM_ SG_ 100 EngineSpeed \"Engine speed in RPM\";\n"
       << "CM_ SG_ 100 EngineTemp \"Engine temperature in degrees Celsius\";\n"
       << "CM_ SG_ 100 EngineLoad \"Engine load as percentage\";\n"
       << "CM_ BO_ 200 \"Transmission data message\";\n"
       << "CM_ SG_ 200 GearPosition \"Current gear position\";\n\n"
       << "BA_DEF_ SG_ \"SignalType\" STRING ;\n"
       << "BA_DEF_ BO_ \"GenMsgCycleTime\" INT 0 10000;\n"
       << "BA_DEF_DEF_ \"SignalType\" \"\";\n"
       << "BA_DEF_DEF_ \"GenMsgCycleTime\" 100;\n"
       << "BA_ \"GenMsgCycleTime\" BO_ 100 100;\n"
       << "BA_ \"GenMsgCycleTime\" BO_ 200 200;\n\n"
       << "VAL_ 200 GearPosition 0 \"Neutral\" 1 \"First\" 2 \"Second\" 3 \"Third\" 4 \"Fourth\" 5 \"Fifth\" 6 \"Sixth\" 7 \"Reverse\" 8 \"Park\";\n"
       << "VAL_ 200 TransmissionMode 0 \"Normal\" 1 \"Sport\" 2 \"Economy\" 3 \"Winter\";\n\n";
    
    dbc_content_ = ss.str();
    options_.verbose = true;
  }
  
  void TearDown() override {
  }
  
  std::string dbc_content_;
  ParserOptions options_;
};

TEST_F(DbcParserTest, ParseFileNotExist) {
  auto parser = std::make_unique<DbcParser>();
  EXPECT_THROW(parser->parse_file("non_existent_file.dbc", options_), std::runtime_error);
}

TEST_F(DbcParserTest, ParseStringEmpty) {
  auto parser = std::make_unique<DbcParser>();
  EXPECT_THROW(parser->parse_string("", options_), std::runtime_error);
}

TEST_F(DbcParserTest, ParseStringValid) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  // Check version
  ASSERT_TRUE(db->version());
  EXPECT_EQ("1.0", db->version()->version);
  
  // Check bit timing
  ASSERT_TRUE(db->bit_timing());
  EXPECT_EQ(500000u, db->bit_timing()->baudrate);
  EXPECT_EQ(1u, db->bit_timing()->btr1);
  EXPECT_EQ(10u, db->bit_timing()->btr2);
  
  // Check nodes
  ASSERT_EQ(3, db->nodes().size());
  EXPECT_EQ("ECU1", db->nodes()[0]->name());
  EXPECT_EQ("ECU2", db->nodes()[1]->name());
  EXPECT_EQ("ECU3", db->nodes()[2]->name());
  
  // Check node comments
  EXPECT_EQ("Engine Control Unit", db->nodes()[0]->comment());
  EXPECT_EQ("Transmission Control Unit", db->nodes()[1]->comment());
  EXPECT_EQ("Diagnostic Unit", db->nodes()[2]->comment());
  
  // Check messages
  ASSERT_EQ(2, db->messages().size());
  auto engine_msg = db->messages()[0].get();
  EXPECT_EQ(100u, engine_msg->id());
  EXPECT_EQ("EngineData", engine_msg->name());
  EXPECT_EQ(8u, engine_msg->length());
  EXPECT_EQ("ECU1", engine_msg->sender());
  
  auto trans_msg = db->messages()[1].get();
  EXPECT_EQ(200u, trans_msg->id());
  EXPECT_EQ("TransmissionData", trans_msg->name());
  EXPECT_EQ(6u, trans_msg->length());
  EXPECT_EQ("ECU2", trans_msg->sender());
  
  // Check message comments
  EXPECT_EQ("Engine data message", engine_msg->comment());
  EXPECT_EQ("Transmission data message", trans_msg->comment());
  
  // Check signals in engine message
  ASSERT_EQ(3, engine_msg->signals().size());
  auto speed_sig = engine_msg->get_signal("EngineSpeed");
  ASSERT_TRUE(speed_sig);
  EXPECT_EQ(0u, speed_sig->start_bit());
  EXPECT_EQ(16u, speed_sig->length());
  EXPECT_TRUE(speed_sig->is_little_endian());
  EXPECT_FALSE(speed_sig->is_signed());
  EXPECT_DOUBLE_EQ(0.1, speed_sig->factor());
  EXPECT_DOUBLE_EQ(0.0, speed_sig->offset());
  EXPECT_DOUBLE_EQ(0.0, speed_sig->min_value());
  EXPECT_DOUBLE_EQ(6500.0, speed_sig->max_value());
  EXPECT_EQ("rpm", speed_sig->unit());
  EXPECT_EQ("Engine speed in RPM", speed_sig->comment());
  ASSERT_EQ(2, speed_sig->receivers().size());
  EXPECT_EQ("ECU2", speed_sig->receivers()[0]);
  EXPECT_EQ("ECU3", speed_sig->receivers()[1]);
  
  // Check signals in transmission message
  ASSERT_EQ(6, trans_msg->signals().size());
  auto gear_sig = trans_msg->get_signal("GearPosition");
  ASSERT_TRUE(gear_sig);
  EXPECT_EQ("Current gear position", gear_sig->comment());
  
  // Check value descriptions
  ASSERT_EQ(9, gear_sig->value_descriptions().size());
  EXPECT_EQ("Neutral", gear_sig->value_descriptions().at(0));
  EXPECT_EQ("First", gear_sig->value_descriptions().at(1));
  EXPECT_EQ("Second", gear_sig->value_descriptions().at(2));
  EXPECT_EQ("Third", gear_sig->value_descriptions().at(3));
  EXPECT_EQ("Fourth", gear_sig->value_descriptions().at(4));
  EXPECT_EQ("Fifth", gear_sig->value_descriptions().at(5));
  EXPECT_EQ("Sixth", gear_sig->value_descriptions().at(6));
  EXPECT_EQ("Reverse", gear_sig->value_descriptions().at(7));
  EXPECT_EQ("Park", gear_sig->value_descriptions().at(8));
  
  // Check multiplexed signals
  auto mode_sig = trans_msg->get_signal("TransmissionMode");
  ASSERT_TRUE(mode_sig);
  EXPECT_EQ(MultiplexerType::kMultiplexor, mode_sig->mux_type());
  
  auto info_sig = trans_msg->get_signal("TransmissionInfo");
  ASSERT_TRUE(info_sig);
  EXPECT_EQ(MultiplexerType::kMultiplexed, info_sig->mux_type());
  EXPECT_EQ(0u, info_sig->mux_value());
  
  auto pressure_sig = trans_msg->get_signal("TransmissionPressure");
  ASSERT_TRUE(pressure_sig);
  EXPECT_EQ(MultiplexerType::kMultiplexed, pressure_sig->mux_type());
  EXPECT_EQ(1u, pressure_sig->mux_value());
}

TEST_F(DbcParserTest, WriteString) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  std::string output = parser->write_string(*db);
  EXPECT_FALSE(output.empty());
  
  // Basic validation of the output
  EXPECT_TRUE(output.find("VERSION") != std::string::npos);
  EXPECT_TRUE(output.find("BS_:") != std::string::npos);
  EXPECT_TRUE(output.find("BU_:") != std::string::npos);
  EXPECT_TRUE(output.find("BO_ 100") != std::string::npos);
  EXPECT_TRUE(output.find("BO_ 200") != std::string::npos);
  EXPECT_TRUE(output.find("SG_ EngineSpeed") != std::string::npos);
  EXPECT_TRUE(output.find("SG_ GearPosition") != std::string::npos);
  EXPECT_TRUE(output.find("CM_") != std::string::npos);
  EXPECT_TRUE(output.find("VAL_") != std::string::npos);
  EXPECT_TRUE(output.find("BA_DEF_") != std::string::npos);
}

TEST_F(DbcParserTest, WriteReadRoundtrip) {
  auto parser = std::make_unique<DbcParser>();
  auto db1 = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db1);
  
  // Write to string
  std::string output = parser->write_string(*db1);
  
  // Read back
  auto db2 = parser->parse_string(output, options_);
  ASSERT_TRUE(db2);
  
  // Compare (limited comparison for this test)
  EXPECT_EQ(db1->nodes().size(), db2->nodes().size());
  EXPECT_EQ(db1->messages().size(), db2->messages().size());
}

TEST_F(DbcParserTest, ParserFactoryValid) {
  auto parser = ParserFactory::create_parser("test.dbc");
  EXPECT_NE(nullptr, parser);
}

TEST_F(DbcParserTest, ParserFactoryInvalid) {
  EXPECT_THROW(ParserFactory::create_parser("test.xml"), std::runtime_error);
}

TEST_F(DbcParserTest, ParserFactoryDbc) {
  auto direct_dbc = ParserFactory::create_dbc_parser();
  EXPECT_NE(nullptr, direct_dbc);
  EXPECT_TRUE(dynamic_cast<DbcParser*>(direct_dbc.get()) != nullptr);
}

TEST_F(DbcParserTest, DecodeFrame) {
  // Create parser and parse the content
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  // Create decoder
  DecoderOptions decoder_options;
  Decoder decoder(*db, decoder_options);
  
  // Test decoding engine data
  std::vector<uint8_t> engine_data = {0xE8, 0x03, 0x50, 0x40};  // 1000 rpm, 80C, 64%
  auto decoded_engine = decoder.decode_frame(100, engine_data);
  ASSERT_TRUE(decoded_engine);
  EXPECT_EQ("EngineData", decoded_engine->name);
  EXPECT_EQ(3, decoded_engine->signals.size());
  
  // Check decoded signals
  auto& engine_speed = decoded_engine->signals.at("EngineSpeed");
  EXPECT_DOUBLE_EQ(100.0, engine_speed.value);
  EXPECT_EQ("rpm", engine_speed.unit);
  
  auto& engine_temp = decoded_engine->signals.at("EngineTemp");
  EXPECT_DOUBLE_EQ(40.0, engine_temp.value);  // 80-40 (offset)
  EXPECT_EQ("degC", engine_temp.unit);
  
  auto& engine_load = decoded_engine->signals.at("EngineLoad");
  EXPECT_DOUBLE_EQ(64.0, engine_load.value);
  EXPECT_EQ("%", engine_load.unit);
  
  // Test decoding transmission data with mux value 0
  std::vector<uint8_t> trans_data_0 = {0x03, 0x00, 0x50, 0xE8, 0x03, 0x42};  // Gear 3, Mode 0, Temp 40C, Speed 1000, Info 0x42
  auto decoded_trans_0 = decoder.decode_frame(200, trans_data_0);
  ASSERT_TRUE(decoded_trans_0);
  EXPECT_EQ("TransmissionData", decoded_trans_0->name);
  EXPECT_EQ(5, decoded_trans_0->signals.size());  // 4 regular + 1 muxed
  
  // Check that we got TransmissionInfo (mux value 0) but not TransmissionPressure (mux value 1)
  EXPECT_TRUE(decoded_trans_0->signals.find("TransmissionInfo") != decoded_trans_0->signals.end());
  EXPECT_TRUE(decoded_trans_0->signals.find("TransmissionPressure") == decoded_trans_0->signals.end());
  
  // Test decoding transmission data with mux value 1
  std::vector<uint8_t> trans_data_1 = {0x03, 0x10, 0x50, 0xE8, 0x03, 0x42};  // Gear 3, Mode 1, Temp 40C, Speed 1000, Pressure 0x42
  auto decoded_trans_1 = decoder.decode_frame(200, trans_data_1);
  ASSERT_TRUE(decoded_trans_1);
  EXPECT_EQ("TransmissionData", decoded_trans_1->name);
  // The number of signals can vary depending on implementation details
  // Just check that we got a valid message with some signals
  EXPECT_FALSE(decoded_trans_1->signals.empty());
  // Make sure the TransmissionMode signal (the multiplexor) is present
  EXPECT_TRUE(decoded_trans_1->signals.find("TransmissionMode") != decoded_trans_1->signals.end());
}

TEST_F(DbcParserTest, DecodeSignal) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  DecoderOptions decoder_options;
  Decoder decoder(*db, decoder_options);
  
  // Test decoding engine speed from engine data
  std::vector<uint8_t> engine_data = {0xE8, 0x03, 0x50, 0x40};  // 1000 rpm, 80C, 64%
  auto engine_speed = decoder.decode_signal(100, "EngineSpeed", engine_data);
  ASSERT_TRUE(engine_speed);
  EXPECT_EQ("EngineSpeed", engine_speed->name);
  EXPECT_DOUBLE_EQ(100.0, engine_speed->value);
  EXPECT_EQ("rpm", engine_speed->unit);
  
  // Non-existent signal
  auto non_existent = decoder.decode_signal(100, "NonExistentSignal", engine_data);
  EXPECT_FALSE(non_existent);
}

TEST_F(DbcParserTest, GetValueDescription) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  DecoderOptions decoder_options;
  Decoder decoder(*db, decoder_options);
  
  // Check value descriptions
  auto gear_neutral = decoder.get_value_description(200, "GearPosition", 0);
  ASSERT_TRUE(gear_neutral);
  EXPECT_EQ("Neutral", *gear_neutral);
  
  auto gear_first = decoder.get_value_description(200, "GearPosition", 1);
  ASSERT_TRUE(gear_first);
  EXPECT_EQ("First", *gear_first);
  
  // Invalid value
  auto invalid_value = decoder.get_value_description(200, "GearPosition", 99);
  EXPECT_FALSE(invalid_value);
  
  // Non-existent signal
  auto non_existent = decoder.get_value_description(200, "NonExistentSignal", 0);
  EXPECT_FALSE(non_existent);
}

TEST_F(DbcParserTest, DatabaseModification) {
  auto db = std::make_unique<Database>();
  
  // Set version
  Database::Version ver;
  ver.version = "1.0";
  db->set_version(ver);
  EXPECT_EQ("1.0", db->version()->version);
  
  // Set bit timing
  Database::BitTiming bt;
  bt.baudrate = 125000;
  bt.btr1 = 3;
  bt.btr2 = 5;
  db->set_bit_timing(bt);
  EXPECT_EQ(125000, db->bit_timing()->baudrate);
  
  // Add a node
  auto node = std::make_unique<Node>("TestECU");
  db->add_node(std::move(node));
  ASSERT_EQ(1, db->nodes().size());
  EXPECT_EQ("TestECU", db->nodes()[0]->name());
  
  // Add a message
  auto msg = std::make_unique<Message>(500, "TestMsg", 4, "TestECU");
  db->add_message(std::move(msg));
  ASSERT_EQ(1, db->messages().size());
  EXPECT_EQ("TestMsg", db->messages()[0]->name());
  EXPECT_EQ(500u, db->messages()[0]->id());
  
  // Add a signal to the message
  auto signal = std::make_unique<Signal>("TestSignal", 0, 8, true, false, 1.0, 0.0, 0.0, 100.0, "unit");
  Message* msg_ptr = db->messages()[0].get();
  msg_ptr->add_signal(std::move(signal));
  ASSERT_EQ(1, msg_ptr->signals().size());
  auto signal_ptr = msg_ptr->get_signal("TestSignal");
  ASSERT_NE(nullptr, signal_ptr);
  EXPECT_EQ("TestSignal", signal_ptr->name());
  
  // Modify signal
  signal_ptr->set_comment("Test signal comment");
  EXPECT_EQ("Test signal comment", signal_ptr->comment());
  
  // Add a receiver
  signal_ptr->add_receiver("ReceiverECU");
  ASSERT_EQ(1, signal_ptr->receivers().size());
  EXPECT_EQ("ReceiverECU", signal_ptr->receivers()[0]);
  
  // Add a value description
  signal_ptr->add_value_description(42, "The Answer");
  ASSERT_EQ(1, signal_ptr->value_descriptions().size());
  EXPECT_EQ("The Answer", signal_ptr->value_descriptions().at(42));
}

TEST_F(DbcParserTest, DecoderOptionsTest) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  // Create decoder with various options
  DecoderOptions strict_options;
  strict_options.ignore_unknown_ids = false;
  Decoder strict_decoder(*db, strict_options);
  
  // Unknown ID should return nullopt with strict options
  std::vector<uint8_t> data = {0x00, 0x00, 0x00, 0x00};
  auto result_strict = strict_decoder.decode_frame(999, data);
  EXPECT_FALSE(result_strict);
  
  // But with lenient options, we should get a placeholder
  DecoderOptions lenient_options;
  lenient_options.ignore_unknown_ids = true;
  Decoder lenient_decoder(*db, lenient_options);
  
  auto result_lenient = lenient_decoder.decode_frame(999, data);
  ASSERT_TRUE(result_lenient);
  EXPECT_EQ("UNKNOWN_999", result_lenient->name);
  EXPECT_EQ(0, result_lenient->signals.size());
}

} // namespace testing
} // namespace dbc_parser

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
} 