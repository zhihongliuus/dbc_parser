#include "../../include/dbc_parser/parser.h"
#include "../../include/dbc_parser/decoder.h"
#include "../../include/dbc_parser/types.h"

#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <fstream>

namespace dbc_parser {
namespace testing {

class DbcParserTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Load test DBC file
    std::ifstream file("test/dbc_parser/test.dbc");
    ASSERT_TRUE(file.is_open()) << "Failed to open test.dbc file";
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    dbc_content_ = buffer.str();
    
    options_.verbose = true;
  }
  
  void TearDown() override {}
  
  std::string dbc_content_;
  ParserOptions options_;
};

TEST_F(DbcParserTest, ParseVersion) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  ASSERT_TRUE(db->version());
  EXPECT_EQ("1.0", db->version()->version);
}

TEST_F(DbcParserTest, ParseNewSymbols) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  EXPECT_FALSE(db->new_symbols().empty());
  EXPECT_TRUE(std::find(db->new_symbols().begin(), db->new_symbols().end(), "CM_") != db->new_symbols().end());
  EXPECT_TRUE(std::find(db->new_symbols().begin(), db->new_symbols().end(), "BA_DEF_") != db->new_symbols().end());
}

TEST_F(DbcParserTest, ParseBitTiming) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  ASSERT_TRUE(db->bit_timing());
  EXPECT_EQ(500000u, db->bit_timing()->baudrate);
  EXPECT_EQ(1u, db->bit_timing()->btr1);
  EXPECT_EQ(10u, db->bit_timing()->btr2);
}

TEST_F(DbcParserTest, ParseNodes) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  ASSERT_EQ(3, db->nodes().size());
  EXPECT_EQ("ECU1", db->nodes()[0]->name());
  EXPECT_EQ("ECU2", db->nodes()[1]->name());
  EXPECT_EQ("ECU3", db->nodes()[2]->name());
  
  EXPECT_EQ("Engine Control Unit", db->nodes()[0]->comment());
  EXPECT_EQ("Transmission Control Unit", db->nodes()[1]->comment());
  EXPECT_EQ("Diagnostic Unit", db->nodes()[2]->comment());
}

TEST_F(DbcParserTest, ParseValueTables) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  ASSERT_EQ(2, db->value_tables().size());
  
  EXPECT_EQ("GearPositions", db->value_tables()[0]->name());
  EXPECT_EQ(9, db->value_tables()[0]->values().size());
  EXPECT_EQ("Neutral", db->value_tables()[0]->values().at(0));
  EXPECT_EQ("First", db->value_tables()[0]->values().at(1));
  
  EXPECT_EQ("TransmissionModes", db->value_tables()[1]->name());
  EXPECT_EQ(4, db->value_tables()[1]->values().size());
  EXPECT_EQ("Normal", db->value_tables()[1]->values().at(0));
  EXPECT_EQ("Sport", db->value_tables()[1]->values().at(1));
}

TEST_F(DbcParserTest, ParseMessages) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  ASSERT_EQ(2, db->messages().size());
  
  auto engine_msg = db->messages()[0].get();
  EXPECT_EQ(100u, engine_msg->id());
  EXPECT_EQ("EngineData", engine_msg->name());
  EXPECT_EQ(8u, engine_msg->length());
  EXPECT_EQ("ECU1", engine_msg->sender());
  EXPECT_EQ("Engine data message", engine_msg->comment());
  
  auto trans_msg = db->messages()[1].get();
  EXPECT_EQ(200u, trans_msg->id());
  EXPECT_EQ("TransmissionData", trans_msg->name());
  EXPECT_EQ(6u, trans_msg->length());
  EXPECT_EQ("ECU2", trans_msg->sender());
  EXPECT_EQ("Transmission data message", trans_msg->comment());
}

TEST_F(DbcParserTest, ParseMessageTransmitters) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  auto engine_msg = db->get_message(100);
  ASSERT_TRUE(engine_msg);
  ASSERT_EQ(1, engine_msg->transmitters().size());
  EXPECT_EQ("ECU1", engine_msg->transmitters()[0]);
  
  auto trans_msg = db->get_message(200);
  ASSERT_TRUE(trans_msg);
  ASSERT_EQ(1, trans_msg->transmitters().size());
  EXPECT_EQ("ECU2", trans_msg->transmitters()[0]);
}

TEST_F(DbcParserTest, ParseSignals) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  auto engine_msg = db->get_message(100);
  ASSERT_TRUE(engine_msg);
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
  
  auto trans_msg = db->get_message(200);
  ASSERT_TRUE(trans_msg);
  ASSERT_EQ(6, trans_msg->signals().size());
  
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

TEST_F(DbcParserTest, ParseEnvironmentVariables) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  ASSERT_EQ(1, db->environment_variables().size());
  
  auto env_var = db->environment_variables()[0].get();
  EXPECT_EQ("EnvTemperature", env_var->name());
  EXPECT_EQ(EnvVarType::kInteger, env_var->type());
  EXPECT_DOUBLE_EQ(0.0, env_var->min_value());
  EXPECT_DOUBLE_EQ(100.0, env_var->max_value());
  EXPECT_EQ("degC", env_var->unit());
  EXPECT_DOUBLE_EQ(25.0, env_var->initial_value());
  
  // Environment variable data
  ASSERT_FALSE(env_var->data_values().empty());
}

TEST_F(DbcParserTest, ParseSignalTypes) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  ASSERT_FALSE(db->signal_types().empty());
  
  EXPECT_EQ("EngineSpeedType", db->signal_types()[0]->name());
}

TEST_F(DbcParserTest, ParseComments) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  // Node comments
  EXPECT_EQ("Engine Control Unit", db->nodes()[0]->comment());
  EXPECT_EQ("Transmission Control Unit", db->nodes()[1]->comment());
  EXPECT_EQ("Diagnostic Unit", db->nodes()[2]->comment());
  
  // Message comments
  auto engine_msg = db->get_message(100);
  ASSERT_TRUE(engine_msg);
  EXPECT_EQ("Engine data message", engine_msg->comment());
  
  auto trans_msg = db->get_message(200);
  ASSERT_TRUE(trans_msg);
  EXPECT_EQ("Transmission data message", trans_msg->comment());
  
  // Signal comments
  auto speed_sig = engine_msg->get_signal("EngineSpeed");
  ASSERT_TRUE(speed_sig);
  EXPECT_EQ("Engine speed in RPM", speed_sig->comment());
  
  auto temp_sig = engine_msg->get_signal("EngineTemp");
  ASSERT_TRUE(temp_sig);
  EXPECT_EQ("Engine temperature in degrees Celsius", temp_sig->comment());
}

TEST_F(DbcParserTest, ParseAttributeDefinitions) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  ASSERT_FALSE(db->attribute_definitions().empty());
  
  bool found_signal_type = false;
  bool found_cycle_time = false;
  
  for (const auto& attr_def : db->attribute_definitions()) {
    if (attr_def->name() == "SignalType") {
      found_signal_type = true;
      EXPECT_EQ(AttributeType::kString, attr_def->type());
    } else if (attr_def->name() == "GenMsgCycleTime") {
      found_cycle_time = true;
      EXPECT_EQ(AttributeType::kInt, attr_def->type());
      EXPECT_DOUBLE_EQ(0.0, std::get<double>(attr_def->min()));
      EXPECT_DOUBLE_EQ(10000.0, std::get<double>(attr_def->max()));
    }
  }
  
  EXPECT_TRUE(found_signal_type);
  EXPECT_TRUE(found_cycle_time);
}

TEST_F(DbcParserTest, ParseAttributeDefaults) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  ASSERT_FALSE(db->attribute_defaults().empty());
  
  EXPECT_EQ("", std::get<std::string>(db->attribute_defaults().at("SignalType")));
  EXPECT_EQ("100", std::get<std::string>(db->attribute_defaults().at("GenMsgCycleTime")));
  EXPECT_EQ("ECU", std::get<std::string>(db->attribute_defaults().at("NodeType")));
  EXPECT_EQ("CAN", std::get<std::string>(db->attribute_defaults().at("NetworkType")));
}

TEST_F(DbcParserTest, ParseAttributeValues) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  ASSERT_FALSE(db->global_attributes().empty());
  
  EXPECT_EQ("CAN Powertrain", std::get<std::string>(db->global_attributes().at("NetworkType")));
  
  // Message attributes
  auto engine_msg = db->get_message(100);
  ASSERT_TRUE(engine_msg);
  EXPECT_EQ("100", std::get<std::string>(engine_msg->attributes().at("GenMsgCycleTime")));
  
  auto trans_msg = db->get_message(200);
  ASSERT_TRUE(trans_msg);
  EXPECT_EQ("200", std::get<std::string>(trans_msg->attributes().at("GenMsgCycleTime")));
  
  // Node attributes
  auto ecu1 = db->get_node("ECU1");
  ASSERT_TRUE(ecu1);
  EXPECT_EQ("Engine", std::get<std::string>(ecu1->attributes().at("NodeType")));
  
  auto ecu2 = db->get_node("ECU2");
  ASSERT_TRUE(ecu2);
  EXPECT_EQ("Transmission", std::get<std::string>(ecu2->attributes().at("NodeType")));
}

TEST_F(DbcParserTest, ParseValueDescriptions) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  auto trans_msg = db->get_message(200);
  ASSERT_TRUE(trans_msg);
  
  auto gear_sig = trans_msg->get_signal("GearPosition");
  ASSERT_TRUE(gear_sig);
  ASSERT_EQ(9, gear_sig->value_descriptions().size());
  EXPECT_EQ("Neutral", gear_sig->value_descriptions().at(0));
  EXPECT_EQ("First", gear_sig->value_descriptions().at(1));
  EXPECT_EQ("Second", gear_sig->value_descriptions().at(2));
  
  auto mode_sig = trans_msg->get_signal("TransmissionMode");
  ASSERT_TRUE(mode_sig);
  ASSERT_EQ(4, mode_sig->value_descriptions().size());
  EXPECT_EQ("Normal", mode_sig->value_descriptions().at(0));
  EXPECT_EQ("Sport", mode_sig->value_descriptions().at(1));
}

TEST_F(DbcParserTest, ParseSignalExtendedValueType) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  auto engine_msg = db->get_message(100);
  ASSERT_TRUE(engine_msg);
  
  auto speed_sig = engine_msg->get_signal("EngineSpeed");
  ASSERT_TRUE(speed_sig);
  EXPECT_EQ(SignalExtendedValueType::kFloat, speed_sig->extended_value_type());
  
  auto temp_sig = engine_msg->get_signal("EngineTemp");
  ASSERT_TRUE(temp_sig);
  EXPECT_EQ(SignalExtendedValueType::kFloat, temp_sig->extended_value_type());
}

TEST_F(DbcParserTest, ParseSignalGroups) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  auto engine_msg = db->get_message(100);
  ASSERT_TRUE(engine_msg);
  ASSERT_EQ(1, engine_msg->signal_groups().size());
  
  auto engine_group = engine_msg->signal_groups()[0].get();
  EXPECT_EQ("EngineGroup", engine_group->name());
  EXPECT_EQ(1u, engine_group->id());
  ASSERT_EQ(3, engine_group->signals().size());
  EXPECT_EQ("EngineSpeed", engine_group->signals()[0]);
  EXPECT_EQ("EngineTemp", engine_group->signals()[1]);
  EXPECT_EQ("EngineLoad", engine_group->signals()[2]);
  
  auto trans_msg = db->get_message(200);
  ASSERT_TRUE(trans_msg);
  ASSERT_EQ(1, trans_msg->signal_groups().size());
  
  auto trans_group = trans_msg->signal_groups()[0].get();
  EXPECT_EQ("TransmissionGroup", trans_group->name());
  EXPECT_EQ(1u, trans_group->id());
  ASSERT_EQ(4, trans_group->signals().size());
}

TEST_F(DbcParserTest, ParseSignalMultiplexerValue) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  auto trans_msg = db->get_message(200);
  ASSERT_TRUE(trans_msg);
  
  auto info_sig = trans_msg->get_signal("TransmissionInfo");
  ASSERT_TRUE(info_sig);
  EXPECT_EQ(MultiplexerType::kMultiplexed, info_sig->mux_type());
  EXPECT_EQ(0u, info_sig->mux_value());
  
  auto pressure_sig = trans_msg->get_signal("TransmissionPressure");
  ASSERT_TRUE(pressure_sig);
  EXPECT_EQ(MultiplexerType::kMultiplexed, pressure_sig->mux_type());
  EXPECT_EQ(1u, pressure_sig->mux_value());
}

TEST_F(DbcParserTest, ParseSignalTypeRefs) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  ASSERT_FALSE(db->signal_type_refs().empty());
  
  EXPECT_EQ("EngineSpeedType", db->signal_type_refs().at(100).at("EngineSpeed"));
}

TEST_F(DbcParserTest, DecodeFrames) {
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(dbc_content_, options_);
  ASSERT_TRUE(db);
  
  // Create decoder
  DecoderOptions decoder_options;
  Decoder decoder(*db, decoder_options);
  
  // Test decoding engine data
  std::vector<uint8_t> engine_data = {0xE8, 0x03, 0x50, 0x40, 0x00, 0x00, 0x00, 0x00};  // 1000 rpm, 80C, 64%
  auto decoded_engine = decoder.decode_frame(100, engine_data);
  ASSERT_TRUE(decoded_engine);
  EXPECT_EQ("EngineData", decoded_engine->name);
  EXPECT_EQ(3, decoded_engine->signals.size());
  
  // Check decoded signals
  auto& engine_speed = decoded_engine->signals.at("EngineSpeed");
  EXPECT_DOUBLE_EQ(100.0, engine_speed.value);  // 0x03E8 * 0.1 = 100.0
  EXPECT_EQ("rpm", engine_speed.unit);
  
  auto& engine_temp = decoded_engine->signals.at("EngineTemp");
  EXPECT_DOUBLE_EQ(40.0, engine_temp.value);  // 80-40 (offset)
  EXPECT_EQ("degC", engine_temp.unit);
  
  auto& engine_load = decoded_engine->signals.at("EngineLoad");
  EXPECT_DOUBLE_EQ(64.0, engine_load.value);
  EXPECT_EQ("%", engine_load.unit);
}

} // namespace testing
} // namespace dbc_parser

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
} 