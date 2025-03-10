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
    // Create test DBC content
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
      "    BA_DEF_DEF_\n"
      "    EV_DATA_\n"
      "    ENVVAR_DATA_\n"
      "    SGTYPE_\n"
      "    SGTYPE_VAL_\n"
      "    BA_DEF_SGTYPE_\n"
      "    BA_SGTYPE_\n"
      "    SIG_TYPE_REF_\n"
      "    VAL_TABLE_\n"
      "    SIG_GROUP_\n"
      "    SIG_VALTYPE_\n"
      "    SIGTYPE_VALTYPE_\n"
      "    BO_TX_BU_\n"
      "    BA_DEF_REL_\n"
      "    BA_REL_\n"
      "    BA_DEF_DEF_REL_\n"
      "    BU_SG_REL_\n"
      "    BU_EV_REL_\n"
      "    BU_BO_REL_\n"
      "    SG_MUL_VAL_\n"
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

    // Create invalid DBC content for error testing
    invalid_dbc_content_ = "This is not a valid DBC file format";

    // Empty DBC content
    empty_dbc_content_ = "";
  }
  
  void TearDown() override {
  }
  
  std::string dbc_content_;
  std::string invalid_dbc_content_;
  std::string empty_dbc_content_;
  ParserOptions options_;
};

TEST_F(DbcParserTest, ParseFromValidString) {
  // Create parser
  auto parser = std::make_unique<DbcParser>();
  
  // Parse the content
  auto db = parser->parse_string(dbc_content_, options_);
  
  // Verify the database was created
  ASSERT_TRUE(db);
  
  // Check version
  ASSERT_TRUE(db->version());
  EXPECT_EQ("1.0", db->version()->version);
  
  // Check bit timing
  ASSERT_TRUE(db->bit_timing());
  EXPECT_EQ(500000, db->bit_timing()->baudrate);
  EXPECT_EQ(1, db->bit_timing()->btr1);
  EXPECT_EQ(10, db->bit_timing()->btr2);
  
  // Check nodes
  EXPECT_EQ(3, db->nodes().size());
  ASSERT_NE(nullptr, db->get_node("ECU1"));
  ASSERT_NE(nullptr, db->get_node("ECU2"));
  ASSERT_NE(nullptr, db->get_node("ECU3"));
  
  // Check messages
  EXPECT_EQ(2, db->messages().size());
  
  // Check EngineData message
  auto engine_msg = db->get_message(100);
  ASSERT_NE(nullptr, engine_msg);
  EXPECT_EQ("EngineData", engine_msg->name());
  EXPECT_EQ(8, engine_msg->length());
  EXPECT_EQ("ECU1", engine_msg->sender());
  EXPECT_EQ("Engine data message", engine_msg->comment());
  EXPECT_EQ(3, engine_msg->signals().size());
  
  // Check EngineSpeed signal
  auto engine_speed = engine_msg->get_signal("EngineSpeed");
  ASSERT_NE(nullptr, engine_speed);
  EXPECT_EQ(0, engine_speed->start_bit());
  EXPECT_EQ(16, engine_speed->length());
  EXPECT_TRUE(engine_speed->is_little_endian());
  EXPECT_FALSE(engine_speed->is_signed());
  EXPECT_DOUBLE_EQ(0.1, engine_speed->factor());
  EXPECT_DOUBLE_EQ(0.0, engine_speed->offset());
  EXPECT_DOUBLE_EQ(0.0, engine_speed->min_value());
  EXPECT_DOUBLE_EQ(6500.0, engine_speed->max_value());
  EXPECT_EQ("rpm", engine_speed->unit());
  EXPECT_EQ("Engine speed in RPM", engine_speed->comment());
  EXPECT_EQ(MultiplexerType::kNone, engine_speed->mux_type());
  ASSERT_EQ(2, engine_speed->receivers().size());
  EXPECT_EQ("ECU2", engine_speed->receivers()[0]);
  EXPECT_EQ("ECU3", engine_speed->receivers()[1]);
  
  // Check TransmissionData message
  auto trans_msg = db->get_message(200);
  ASSERT_NE(nullptr, trans_msg);
  EXPECT_EQ("TransmissionData", trans_msg->name());
  EXPECT_EQ(6, trans_msg->length());
  EXPECT_EQ("ECU2", trans_msg->sender());
  EXPECT_EQ("Transmission data message", trans_msg->comment());
  EXPECT_EQ(6, trans_msg->signals().size());
  
  // Check TransmissionMode signal (multiplexor)
  auto trans_mode = trans_msg->get_signal("TransmissionMode");
  ASSERT_NE(nullptr, trans_mode);
  EXPECT_EQ(MultiplexerType::kMultiplexor, trans_mode->mux_type());
  
  // Check multiplexed signals
  auto trans_info = trans_msg->get_signal("TransmissionInfo");
  ASSERT_NE(nullptr, trans_info);
  EXPECT_EQ(MultiplexerType::kMultiplexed, trans_info->mux_type());
  EXPECT_EQ(0, trans_info->mux_value());
  
  auto trans_pressure = trans_msg->get_signal("TransmissionPressure");
  ASSERT_NE(nullptr, trans_pressure);
  EXPECT_EQ(MultiplexerType::kMultiplexed, trans_pressure->mux_type());
  EXPECT_EQ(1, trans_pressure->mux_value());
  
  // Check value descriptions
  auto gear_pos = trans_msg->get_signal("GearPosition");
  ASSERT_NE(nullptr, gear_pos);
  ASSERT_EQ(9, gear_pos->value_descriptions().size());
  EXPECT_EQ("Neutral", gear_pos->value_descriptions().at(0));
  EXPECT_EQ("First", gear_pos->value_descriptions().at(1));
  EXPECT_EQ("Park", gear_pos->value_descriptions().at(8));
}

TEST_F(DbcParserTest, ParseInvalidString) {
  // Create parser
  auto parser = std::make_unique<DbcParser>();
  
  // Parse invalid content should throw
  EXPECT_THROW(parser->parse_string(invalid_dbc_content_, options_), std::runtime_error);
}

TEST_F(DbcParserTest, ParseEmptyString) {
  // Create parser
  auto parser = std::make_unique<DbcParser>();
  
  // Parse empty content should throw
  EXPECT_THROW(parser->parse_string(empty_dbc_content_, options_), std::runtime_error);
}

TEST_F(DbcParserTest, WriteAndReadBack) {
  // Create parser and parse the content
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  // Write the database to a string
  std::string output = parser->write_string(*db);
  
  // Parse the output string
  auto new_db = parser->parse_string(output, options_);
  ASSERT_TRUE(new_db);
  
  // Verify the new database has the same basic structure
  EXPECT_EQ(db->messages().size(), new_db->messages().size());
  EXPECT_EQ(db->nodes().size(), new_db->nodes().size());
  
  // Verify a message is the same
  auto original_msg = db->get_message(100);
  auto new_msg = new_db->get_message(100);
  ASSERT_NE(nullptr, original_msg);
  ASSERT_NE(nullptr, new_msg);
  EXPECT_EQ(original_msg->name(), new_msg->name());
  EXPECT_EQ(original_msg->length(), new_msg->length());
  EXPECT_EQ(original_msg->signals().size(), new_msg->signals().size());
}

TEST_F(DbcParserTest, ParserFactoryTest) {
  // Test with .dbc extension
  auto dbc_parser = ParserFactory::create_parser("test.dbc");
  EXPECT_NE(nullptr, dbc_parser);
  EXPECT_TRUE(dynamic_cast<DbcParser*>(dbc_parser.get()) != nullptr);
  
  // Test with .kcd extension
  auto kcd_parser = ParserFactory::create_parser("test.kcd");
  EXPECT_NE(nullptr, kcd_parser);
  EXPECT_TRUE(dynamic_cast<KcdParser*>(kcd_parser.get()) != nullptr);
  
  // Test with unknown extension
  auto default_parser = ParserFactory::create_parser("test.unknown");
  EXPECT_NE(nullptr, default_parser);
  EXPECT_TRUE(dynamic_cast<DbcParser*>(default_parser.get()) != nullptr);
  
  // Test direct creation
  auto direct_dbc = ParserFactory::create_dbc_parser();
  EXPECT_NE(nullptr, direct_dbc);
  EXPECT_TRUE(dynamic_cast<DbcParser*>(direct_dbc.get()) != nullptr);
  
  auto direct_kcd = ParserFactory::create_kcd_parser();
  EXPECT_NE(nullptr, direct_kcd);
  EXPECT_TRUE(dynamic_cast<KcdParser*>(direct_kcd.get()) != nullptr);
}

TEST_F(DbcParserTest, DecodeFrame) {
  // Create parser and parse the content
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  // Create decoder
  DecoderOptions decoder_options;
  Decoder decoder(std::make_shared<Database>(*db), decoder_options);
  
  // Test decoding engine data
  std::vector<uint8_t> engine_data = {0xE8, 0x03, 0x50, 0x40};  // 1000 rpm, 80C, 64%
  auto decoded_engine = decoder.decode_frame(100, engine_data);
  ASSERT_TRUE(decoded_engine);
  EXPECT_EQ("EngineData", decoded_engine->name);
  EXPECT_EQ(3, decoded_engine->signals.size());
  
  // Check decoded signals
  ASSERT_TRUE(decoded_engine->signals.find("EngineSpeed") != decoded_engine->signals.end());
  EXPECT_DOUBLE_EQ(100.0, decoded_engine->signals["EngineSpeed"].value);  // 0x03E8 = 1000, 1000 * 0.1 = 100.0
  EXPECT_EQ("rpm", decoded_engine->signals["EngineSpeed"].unit);
  
  ASSERT_TRUE(decoded_engine->signals.find("EngineTemp") != decoded_engine->signals.end());
  EXPECT_DOUBLE_EQ(40.0, decoded_engine->signals["EngineTemp"].value);  // 0x50 = 80, 80 - 40 = 40
  EXPECT_EQ("degC", decoded_engine->signals["EngineTemp"].unit);
  
  ASSERT_TRUE(decoded_engine->signals.find("EngineLoad") != decoded_engine->signals.end());
  EXPECT_DOUBLE_EQ(64.0, decoded_engine->signals["EngineLoad"].value);  // 0x40 = 64
  EXPECT_EQ("%", decoded_engine->signals["EngineLoad"].unit);
  
  // Test decoding transmission data with multiplexing
  std::vector<uint8_t> trans_data = {0x03, 0x10, 0x64, 0x05, 0x00, 0x00};  // Gear 3, Mode 1, Temp 60C, Speed 130rpm, Mode 1 data is pressure = 0
  auto decoded_trans = decoder.decode_frame(200, trans_data);
  ASSERT_TRUE(decoded_trans);
  EXPECT_EQ("TransmissionData", decoded_trans->name);
  EXPECT_EQ(5, decoded_trans->signals.size());  // All except the non-applicable multiplexed signal
  
  // Check a multiplexed signal (TransmissionPressure should be present, TransmissionInfo should not)
  ASSERT_TRUE(decoded_trans->signals.find("TransmissionPressure") != decoded_trans->signals.end());
  EXPECT_DOUBLE_EQ(0.0, decoded_trans->signals["TransmissionPressure"].value);
  EXPECT_EQ("kPa", decoded_trans->signals["TransmissionPressure"].unit);
  
  EXPECT_TRUE(decoded_trans->signals.find("TransmissionInfo") == decoded_trans->signals.end());
  
  // Check a value with description
  ASSERT_TRUE(decoded_trans->signals.find("GearPosition") != decoded_trans->signals.end());
  EXPECT_DOUBLE_EQ(3.0, decoded_trans->signals["GearPosition"].value);
  EXPECT_EQ("Third", decoded_trans->signals["GearPosition"].description);
  
  ASSERT_TRUE(decoded_trans->signals.find("TransmissionMode") != decoded_trans->signals.end());
  EXPECT_DOUBLE_EQ(1.0, decoded_trans->signals["TransmissionMode"].value);
  EXPECT_EQ("Sport", decoded_trans->signals["TransmissionMode"].description);
}

TEST_F(DbcParserTest, DecodeSignal) {
  // Create parser and parse the content
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  // Create decoder
  DecoderOptions decoder_options;
  Decoder decoder(std::make_shared<Database>(*db), decoder_options);
  
  // Test decoding a specific signal
  std::vector<uint8_t> engine_data = {0xE8, 0x03, 0x50, 0x40};
  auto decoded_speed = decoder.decode_signal(100, "EngineSpeed", engine_data);
  ASSERT_TRUE(decoded_speed);
  EXPECT_EQ("EngineSpeed", decoded_speed->name);
  EXPECT_DOUBLE_EQ(100.0, decoded_speed->value);
  EXPECT_EQ("rpm", decoded_speed->unit);
  
  // Test decoding non-existent message/signal
  EXPECT_FALSE(decoder.decode_signal(999, "NonExistentSignal", engine_data));
  EXPECT_FALSE(decoder.decode_signal(100, "NonExistentSignal", engine_data));
}

TEST_F(DbcParserTest, GetValueDescription) {
  // Create parser and parse the content
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  // Create decoder
  DecoderOptions decoder_options;
  Decoder decoder(std::make_shared<Database>(*db), decoder_options);
  
  // Test getting value descriptions
  auto desc = decoder.get_value_description(200, "GearPosition", 3);
  ASSERT_TRUE(desc);
  EXPECT_EQ("Third", *desc);
  
  // Test non-existent value
  EXPECT_FALSE(decoder.get_value_description(200, "GearPosition", 99));
  
  // Test non-existent signal
  EXPECT_FALSE(decoder.get_value_description(200, "NonExistentSignal", 0));
  
  // Test non-existent message
  EXPECT_FALSE(decoder.get_value_description(999, "GearPosition", 0));
}

TEST_F(DbcParserTest, DatabaseModification) {
  // Create a new database
  auto db = std::make_unique<Database>();
  
  // Add a version
  Database::Version ver;
  ver.version = "2.0";
  db->set_version(ver);
  EXPECT_EQ("2.0", db->version()->version);
  
  // Add bit timing
  Database::BitTiming bt;
  bt.baudrate = 125000;
  bt.btr1 = 2;
  bt.btr2 = 5;
  db->set_bit_timing(bt);
  EXPECT_EQ(125000, db->bit_timing()->baudrate);
  
  // Add a node
  auto node = std::make_unique<Node>("TestECU");
  auto node_ptr = db->add_node(std::move(node));
  ASSERT_NE(nullptr, node_ptr);
  EXPECT_EQ("TestECU", node_ptr->name());
  
  // Add a message
  auto msg = std::make_unique<Message>(500, "TestMsg", 4, "TestECU");
  auto msg_ptr = db->add_message(std::move(msg));
  ASSERT_NE(nullptr, msg_ptr);
  EXPECT_EQ("TestMsg", msg_ptr->name());
  EXPECT_EQ(500u, msg_ptr->id());
  
  // Add a signal to the message
  auto signal = std::make_unique<Signal>("TestSignal", 0, 8, true, false, 1.0, 0.0, 0.0, 100.0, "unit");
  auto signal_ptr = msg_ptr->add_signal(std::move(signal));
  ASSERT_NE(nullptr, signal_ptr);
  EXPECT_EQ("TestSignal", signal_ptr->name());
  
  // Modify signal
  signal_ptr->set_factor(2.0);
  signal_ptr->set_offset(10.0);
  EXPECT_DOUBLE_EQ(2.0, signal_ptr->factor());
  EXPECT_DOUBLE_EQ(10.0, signal_ptr->offset());
  
  // Add a receiver
  signal_ptr->add_receiver("ReceiverECU");
  ASSERT_EQ(1, signal_ptr->receivers().size());
  EXPECT_EQ("ReceiverECU", signal_ptr->receivers()[0]);
  
  // Add a value description
  signal_ptr->add_value_description(42, "The Answer");
  ASSERT_EQ(1, signal_ptr->value_descriptions().size());
  EXPECT_EQ("The Answer", signal_ptr->value_descriptions().at(42));
  
  // Remove a message
  EXPECT_TRUE(db->remove_message(500));
  EXPECT_EQ(0, db->messages().size());
  EXPECT_FALSE(db->remove_message(999));  // Non-existent
}

TEST_F(DbcParserTest, DecoderOptionsTest) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  // Create decoder with various options
  DecoderOptions default_options;
  EXPECT_FALSE(default_options.verbose);
  EXPECT_TRUE(default_options.ignore_unknown_ids);
  
  // Test with ignore_unknown_ids = false
  DecoderOptions strict_options;
  strict_options.ignore_unknown_ids = false;
  Decoder strict_decoder(std::make_shared<Database>(*db), strict_options);
  
  // Should return nullopt for unknown message ID
  EXPECT_FALSE(strict_decoder.decode_frame(999, std::vector<uint8_t>{0, 0, 0, 0}));
  
  // Test with ignore_unknown_ids = true
  DecoderOptions lenient_options;
  lenient_options.ignore_unknown_ids = true;
  Decoder lenient_decoder(std::make_shared<Database>(*db), lenient_options);
  
  // Should return a placeholder message for unknown ID
  auto result = lenient_decoder.decode_frame(999, std::vector<uint8_t>{0, 0, 0, 0});
  ASSERT_TRUE(result);
  EXPECT_EQ(999u, result->id);
  EXPECT_EQ("UNKNOWN_999", result->name);
  EXPECT_TRUE(result->signals.empty());
}

TEST_F(DbcParserTest, ParserOptionsTest) {
  // Create parser
  auto parser = std::make_unique<DbcParser>();
  
  // Test default options
  ParserOptions default_options;
  EXPECT_FALSE(default_options.verbose);
  
  // Test with verbose = true
  ParserOptions verbose_options;
  verbose_options.verbose = true;
  auto db = parser->parse_string(dbc_content_, verbose_options);
  ASSERT_TRUE(db);
}

} // namespace testing
} // namespace dbc_parser

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
} 