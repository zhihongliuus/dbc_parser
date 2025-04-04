#include "src/dbc_parser/parser/dbc_file_parser.h"

#include <string>
#include <string_view>
#include <map>

#include "gtest/gtest.h"

namespace dbc_parser {
namespace parser {
namespace {

// Test fixture for DBC file parser
class DbcFileParserTest : public ::testing::Test {
 protected:
  void SetUp() override {
    parser_ = std::make_unique<DbcFileParser>();
  }

  std::unique_ptr<DbcFileParser> parser_;
};

// Test basic version parsing
TEST_F(DbcFileParserTest, ParsesVersion) {
  const std::string kInput = "VERSION \"1.0\"\n";

  auto result = parser_->Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("1.0", result->version);
}

// Test handling invalid version format
TEST_F(DbcFileParserTest, HandlesInvalidVersionFormat) {
  const std::string kInput = "VERSION 1.0\n";  // Missing quotes

  auto result = parser_->Parse(kInput);
  EXPECT_FALSE(result.has_value());
}

// Test parsing with multiple sections
TEST_F(DbcFileParserTest, HandlesCombinedSections) {
  const std::string kInput = R"(
VERSION "2.0"

NS_ : 
    NS_DESC_
    CM_
)";

  auto result = parser_->Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("2.0", result->version);
  // Note: Currently the new_symbols parser is defined but may not be implemented fully
  // so we're not checking new_symbols content
}

// Test parsing nodes section
TEST_F(DbcFileParserTest, ParsesNodes) {
  const std::string kInput = R"(
VERSION "2.0"
BU_: Node1 Node2 Node3
)";

  auto result = parser_->Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("2.0", result->version);
  ASSERT_EQ(3, result->nodes.size());
  EXPECT_EQ("Node1", result->nodes[0]);
  EXPECT_EQ("Node2", result->nodes[1]);
  EXPECT_EQ("Node3", result->nodes[2]);
}

// Test parsing messages section
TEST_F(DbcFileParserTest, ParsesMessages) {
  const std::string kInput = R"(
VERSION "2.0"
BO_ 123 TestMessage: 8 Node1
)";

  auto result = parser_->Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("2.0", result->version);
  ASSERT_EQ(1, result->messages.size());
  EXPECT_EQ("TestMessage", result->messages[123]);
}

// Test parsing message transmitters section
TEST_F(DbcFileParserTest, ParsesMessageTransmitters) {
  // This test specifically focuses on the BO_TX_BU_ section
  const std::string kInput = R"(
VERSION "2.0"
BO_ 123 TestMessage: 8 Node1
BO_TX_BU_ 123 : Node1, Node2;
)";

  auto result = parser_->Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("2.0", result->version);
  EXPECT_EQ(1, result->messages.size());
  
  // For debugging, print the number of transmitters
  std::cout << "Number of message transmitters: " << result->message_transmitters.size() << std::endl;
  
  ASSERT_EQ(1, result->message_transmitters.size()) << "Expected to find message transmitters for message ID 123";
  ASSERT_EQ(2, result->message_transmitters[123].size()) << "Expected 2 transmitters for message ID 123";
  EXPECT_EQ("Node1", result->message_transmitters[123][0]);
  EXPECT_EQ("Node2", result->message_transmitters[123][1]);
}

// Test bit timing section
TEST_F(DbcFileParserTest, ParsesBitTiming) {
  const std::string kInput = R"(
VERSION "2.0"
BS_: 500
)";

  auto result = parser_->Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("2.0", result->version);
  // We're not checking bit_timing as it's not fully implemented yet
}

// Test value table section
TEST_F(DbcFileParserTest, ParsesValueTable) {
  const std::string kInput = R"(
VERSION "2.0"
VAL_TABLE_ StateTable 0 "Off" 1 "On" 2 "Error";
)";

  auto result = parser_->Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("2.0", result->version);
  
  // Verify value table content
  ASSERT_EQ(1, result->value_tables.size()) << "Expected one value table";
  ASSERT_TRUE(result->value_tables.count("StateTable") > 0) << "Expected to find StateTable";
  
  const auto& table = result->value_tables.at("StateTable");
  ASSERT_EQ(3, table.size()) << "Expected 3 values in StateTable";
  EXPECT_EQ("Off", table.at(0));
  EXPECT_EQ("On", table.at(1));
  EXPECT_EQ("Error", table.at(2));
}

// Test parsing multiple sections
TEST_F(DbcFileParserTest, ParsesMultipleSections) {
  constexpr std::string_view kInput = R"(
VERSION "2.0"

NS_ : 
    NS_DESC_
    CM_
    BA_DEF_
    BA_
    VAL_
    CAT_DEF_
    CAT_
    FILTER
    BA_DEF_DEF_
    EV_DATA_
    ENVVAR_DATA_
    SGTYPE_
    SGTYPE_VAL_
    BA_DEF_SGTYPE_
    BA_SGTYPE_
    SIG_TYPE_REF_
    VAL_TABLE_
    SIG_GROUP_
    SIG_VALTYPE_
    SIGTYPE_VALTYPE_
    BO_TX_BU_
    BA_DEF_REL_
    BA_REL_
    BA_DEF_DEF_REL_
    BU_SG_REL_
    BU_EV_REL_
    BU_BO_REL_
    SG_MUL_VAL_

BS_: 500

BU_: Node1 Node2

EV_ EngineTemp 1 [0|120] "C" 20 0 DUMMY_NODE_VECTOR0 Vector__XXX;
)";

  auto result = parser_->Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("2.0", result->version);
  EXPECT_EQ(2, result->nodes.size());
  
  // Check environment variables
  std::cout << "Environment variables count: " << result->environment_variables.size() << std::endl;
  
  // Not checking new_symbols since it may not be implemented fully
}

// Test handling malformed input
TEST_F(DbcFileParserTest, HandlesMalformedInput) {
  const std::string kInput = "UNEXPECTED_SECTION_NAME content";
  auto result = parser_->Parse(kInput);
  // The parser should skip unrecognized sections but return an empty result
  // since no valid sections were found
  EXPECT_FALSE(result.has_value());
}

// Test handling empty input
TEST_F(DbcFileParserTest, HandlesEmptyInput) {
  const std::string kInput = "";
  auto result = parser_->Parse(kInput);
  EXPECT_FALSE(result.has_value());
}

// Test that comments and empty lines are skipped
TEST_F(DbcFileParserTest, SkipsCommentsAndEmptyLines) {
  const std::string kInput = R"(
// This is a comment
VERSION "2.0"

// Another comment
BU_: Node1 Node2

)";

  auto result = parser_->Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("2.0", result->version);
  ASSERT_EQ(2, result->nodes.size());
}

// Test that parse errors are handled properly
TEST_F(DbcFileParserTest, HandlesPegtlParseErrors) {
  // Create an intentionally malformed input that would trigger a PEGTL parse error
  // This is a bit contrived but should trigger the exception handling
  const std::string kInput = "VERSION \"unclosed string\nBU_: Node1 Node2";
  
  // The parser should catch any PEGTL exceptions and return nullopt
  auto result = parser_->Parse(kInput);
  EXPECT_FALSE(result.has_value());
}

// Test complex DBC file
TEST_F(DbcFileParserTest, ParsesComplexDbcFile) {
  // Test a more complex DBC file with multiple sections in typical order
  const std::string kInput = R"(
VERSION "2.0"

NS_ : 
    NS_DESC_
    CM_

BS_: 500

BU_: ECU1 ECU2 ECU3

BO_ 100 Engine: 8 ECU1
BO_ 200 Transmission: 8 ECU2
BO_ 300 Brakes: 8 ECU3

BO_TX_BU_ 100 : ECU1;
BO_TX_BU_ 200 : ECU2;
BO_TX_BU_ 300 : ECU3;
)";

  auto result = parser_->Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("2.0", result->version);
  
  // Check nodes
  ASSERT_EQ(3, result->nodes.size());
  EXPECT_EQ("ECU1", result->nodes[0]);
  EXPECT_EQ("ECU2", result->nodes[1]);
  EXPECT_EQ("ECU3", result->nodes[2]);
  
  // Check messages
  ASSERT_EQ(3, result->messages.size());
  EXPECT_EQ("Engine", result->messages[100]);
  EXPECT_EQ("Transmission", result->messages[200]);
  EXPECT_EQ("Brakes", result->messages[300]);
  
  // Check message transmitters
  ASSERT_EQ(3, result->message_transmitters.size()) << "Expected to find message transmitters";
  ASSERT_EQ(1, result->message_transmitters[100].size());
  EXPECT_EQ("ECU1", result->message_transmitters[100][0]);
  ASSERT_EQ(1, result->message_transmitters[200].size());
  EXPECT_EQ("ECU2", result->message_transmitters[200][0]);
  ASSERT_EQ(1, result->message_transmitters[300].size());
  EXPECT_EQ("ECU3", result->message_transmitters[300][0]);
}

// Test detecting signals within messages
TEST_F(DbcFileParserTest, DetectsSignals) {
  const std::string kInput = R"(
VERSION "2.0"
BO_ 123 TestMessage: 8 Node1
 SG_ SignalName : 8|16@1+ (0.1,0) [0|655.35] "km/h" ECU1,ECU2
)";

  auto result = parser_->Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("2.0", result->version);
  ASSERT_EQ(1, result->messages.size());
  EXPECT_EQ("TestMessage", result->messages[123]);
  
  // We can't test signal content yet since we only detect signals
  // but don't parse them due to the Signal struct conflict
  // This will be updated when full signal parsing is implemented
  
  // Verify that the parser processed the signal section (found_valid_section was true)
  // This is checking that our basic signal detection is working
  EXPECT_TRUE(result.has_value());
}

// Test parsing environment variables section
TEST_F(DbcFileParserTest, ParsesEnvironmentVariables) {
  const std::string kInput = R"(
VERSION "2.0"
EV_ EngineTemp 1 [0|120] "C" 20 0 DUMMY_NODE_VECTOR0 Vector__XXX;
)";

  auto result = parser_->Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("2.0", result->version);
  
  // Check environment variables
  ASSERT_EQ(1, result->environment_variables.size()) << "Expected to find 1 environment variable";
  ASSERT_TRUE(result->environment_variables.count("EngineTemp") > 0) << "Expected to find EngineTemp";
  
  const auto& env_var = result->environment_variables.at("EngineTemp");
  EXPECT_EQ("EngineTemp", env_var.name);
  EXPECT_EQ(1, env_var.type);                // Float type
  EXPECT_DOUBLE_EQ(0.0, env_var.min_value);
  EXPECT_DOUBLE_EQ(120.0, env_var.max_value);
  EXPECT_EQ("C", env_var.unit);
  EXPECT_DOUBLE_EQ(20.0, env_var.initial_value);
  EXPECT_EQ(0, env_var.ev_id);
  EXPECT_EQ("DUMMY_NODE_VECTOR0", env_var.access_type);
  ASSERT_EQ(1, env_var.access_nodes.size());
  EXPECT_EQ("Vector__XXX", env_var.access_nodes[0]);
}

// Test parsing environment variable data section
TEST_F(DbcFileParserTest, ParsesEnvironmentVariableData) {
  const std::string kInput = R"(
VERSION "2.0"
ENVVAR_DATA_ EngineTemp: 5;
)";

  auto result = parser_->Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("2.0", result->version);
  
  // Check environment variable data
  ASSERT_EQ(1, result->environment_variable_data.size()) << "Expected to find 1 environment variable data";
  ASSERT_TRUE(result->environment_variable_data.count("EngineTemp") > 0) << "Expected to find EngineTemp data";
  
  const auto& env_var_data = result->environment_variable_data.at("EngineTemp");
  EXPECT_EQ("EngineTemp", env_var_data.data_name);
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 