#include <gtest/gtest.h>
#include <fstream>
#include <memory>
#include <string>

#include "src/dbc_parser.h"

namespace dbc_parser {
namespace {

TEST(DbcParserTest, Initialization) {
  DbcParser parser;
  EXPECT_EQ("", parser.GetLastError());
}

TEST(DbcParserTest, ParseVersion) {
  // Create a temporary DBC file with a VERSION
  std::string temp_file = "test_version.dbc";
  {
    std::ofstream file(temp_file);
    file << "VERSION \"1.2.3\"" << std::endl;
  }
  
  // Parse the file
  DbcParser parser;
  std::unique_ptr<DbcFile> dbc_file;
  int result = parser.Parse(temp_file, &dbc_file);
  
  // Verify results
  EXPECT_EQ(0, result);
  EXPECT_EQ("1.2.3", dbc_file->GetVersion());
  
  // Clean up
  std::remove(temp_file.c_str());
}

TEST(DbcParserTest, ParseNodes) {
  // Create a temporary DBC file with nodes
  std::string temp_file = "test_nodes.dbc";
  {
    std::ofstream file(temp_file);
    file << "BU_: ECU1 ECU2 ECU3" << std::endl;
  }
  
  // Parse the file
  DbcParser parser;
  std::unique_ptr<DbcFile> dbc_file;
  int result = parser.Parse(temp_file, &dbc_file);
  
  // Verify results
  EXPECT_EQ(0, result);
  const auto& nodes = dbc_file->GetNodes();
  EXPECT_EQ(3, nodes.size());
  EXPECT_EQ("ECU1", nodes[0].name);
  EXPECT_EQ("ECU2", nodes[1].name);
  EXPECT_EQ("ECU3", nodes[2].name);
  
  // Clean up
  std::remove(temp_file.c_str());
}

TEST(DbcParserTest, ParseMessages) {
  // Create a temporary DBC file with messages
  std::string temp_file = "test_messages.dbc";
  {
    std::ofstream file(temp_file);
    file << "BO_ 100 EngineStatus: 8 ECU1" << std::endl;
    file << "BO_ 200 VehicleStatus: 6 ECU2" << std::endl;
  }
  
  // Parse the file
  DbcParser parser;
  std::unique_ptr<DbcFile> dbc_file;
  int result = parser.Parse(temp_file, &dbc_file);
  
  // Verify results
  EXPECT_EQ(0, result);
  const auto& messages = dbc_file->GetMessages();
  EXPECT_EQ(2, messages.size());
  
  // First message
  EXPECT_EQ(100, messages[0].id);
  EXPECT_EQ("EngineStatus", messages[0].name);
  EXPECT_EQ(8, messages[0].dlc);
  EXPECT_EQ("ECU1", messages[0].sender);
  
  // Second message
  EXPECT_EQ(200, messages[1].id);
  EXPECT_EQ("VehicleStatus", messages[1].name);
  EXPECT_EQ(6, messages[1].dlc);
  EXPECT_EQ("ECU2", messages[1].sender);
  
  // Clean up
  std::remove(temp_file.c_str());
}

TEST(DbcParserTest, ParseSimpleSignal) {
  // Create a temporary DBC file with a message and a single signal
  std::string temp_file = "test_simple_signal.dbc";
  {
    std::ofstream file(temp_file);
    file << "BO_ 100 EngineStatus: 8 ECU1" << std::endl;
    file << " SG_ EngineSpeed : 0|16@1+ (0.1,0) [0|6500] \"rpm\" ECU2" << std::endl;
  }
  
  // Parse the file
  DbcParser parser;
  std::unique_ptr<DbcFile> dbc_file;
  int result = parser.Parse(temp_file, &dbc_file);
  
  // Verify results
  EXPECT_EQ(0, result);
  const auto& messages = dbc_file->GetMessages();
  EXPECT_EQ(1, messages.size());
  
  // Check if the message has the signal
  EXPECT_EQ(1, messages[0].signals.size());
  
  // Clean up
  std::remove(temp_file.c_str());
}

TEST(DbcParserTest, ParseSignalDetails) {
  // Create a temporary DBC file with a message and signals with various details
  std::string temp_file = "test_signal_details.dbc";
  {
    std::ofstream file(temp_file);
    file << "BO_ 100 EngineStatus: 8 ECU1" << std::endl;
    file << " SG_ EngineSpeed : 0|16@1+ (0.1,0) [0|6500] \"rpm\" ECU2" << std::endl;
    file << " SG_ EngineTemp : 16|8@1+ (1,-40) [-40|215] \"C\" ECU2" << std::endl;
    file << "BO_ 200 VehicleStatus: 6 ECU2" << std::endl;
    file << " SG_ VehicleSpeed : 0|16@1+ (0.01,0) [0|250] \"km/h\" ECU1 ECU3" << std::endl;
  }
  
  // Parse the file
  DbcParser parser;
  std::unique_ptr<DbcFile> dbc_file;
  int result = parser.Parse(temp_file, &dbc_file);
  
  // Verify results
  EXPECT_EQ(0, result);
  const auto& messages = dbc_file->GetMessages();
  EXPECT_EQ(2, messages.size());
  
  // Check first message signals
  EXPECT_EQ(2, messages[0].signals.size());
  
  // EngineSpeed signal details
  const Signal& engine_speed = messages[0].signals[0];
  EXPECT_EQ("EngineSpeed", engine_speed.name);
  EXPECT_EQ(0, engine_speed.start_bit);
  EXPECT_EQ(16, engine_speed.length);
  EXPECT_EQ(ByteOrder::kBigEndian, engine_speed.byte_order);
  EXPECT_FALSE(engine_speed.is_signed);
  EXPECT_DOUBLE_EQ(0.1, engine_speed.factor);
  EXPECT_DOUBLE_EQ(0.0, engine_speed.offset);
  EXPECT_DOUBLE_EQ(0.0, engine_speed.min_value);
  EXPECT_DOUBLE_EQ(6500.0, engine_speed.max_value);
  EXPECT_EQ("rpm", engine_speed.unit);
  EXPECT_EQ(1, engine_speed.receiver_nodes.size());
  EXPECT_EQ("ECU2", engine_speed.receiver_nodes[0]);
  
  // EngineTemp signal details
  const Signal& engine_temp = messages[0].signals[1];
  EXPECT_EQ("EngineTemp", engine_temp.name);
  EXPECT_EQ(16, engine_temp.start_bit);
  EXPECT_EQ(8, engine_temp.length);
  EXPECT_EQ(ByteOrder::kBigEndian, engine_temp.byte_order);
  EXPECT_FALSE(engine_temp.is_signed);
  EXPECT_DOUBLE_EQ(1.0, engine_temp.factor);
  EXPECT_DOUBLE_EQ(-40.0, engine_temp.offset);
  EXPECT_DOUBLE_EQ(-40.0, engine_temp.min_value);
  EXPECT_DOUBLE_EQ(215.0, engine_temp.max_value);
  EXPECT_EQ("C", engine_temp.unit);
  EXPECT_EQ(1, engine_temp.receiver_nodes.size());
  EXPECT_EQ("ECU2", engine_temp.receiver_nodes[0]);
  
  // Check second message signals
  EXPECT_EQ(1, messages[1].signals.size());
  
  // VehicleSpeed signal details
  const Signal& vehicle_speed = messages[1].signals[0];
  EXPECT_EQ("VehicleSpeed", vehicle_speed.name);
  EXPECT_EQ(0, vehicle_speed.start_bit);
  EXPECT_EQ(16, vehicle_speed.length);
  EXPECT_EQ(ByteOrder::kBigEndian, vehicle_speed.byte_order);
  EXPECT_FALSE(vehicle_speed.is_signed);
  EXPECT_DOUBLE_EQ(0.01, vehicle_speed.factor);
  EXPECT_DOUBLE_EQ(0.0, vehicle_speed.offset);
  EXPECT_DOUBLE_EQ(0.0, vehicle_speed.min_value);
  EXPECT_DOUBLE_EQ(250.0, vehicle_speed.max_value);
  EXPECT_EQ("km/h", vehicle_speed.unit);
  EXPECT_EQ(2, vehicle_speed.receiver_nodes.size());
  EXPECT_EQ("ECU1", vehicle_speed.receiver_nodes[0]);
  EXPECT_EQ("ECU3", vehicle_speed.receiver_nodes[1]);
  
  // Clean up
  std::remove(temp_file.c_str());
}

// More tests will be added as the implementation progresses

}  // namespace
}  // namespace dbc_parser 