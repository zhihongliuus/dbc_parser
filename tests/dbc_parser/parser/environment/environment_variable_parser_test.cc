#include "src/dbc_parser/parser/environment/environment_variable_parser.h"

#include <gtest/gtest.h>
#include <optional>
#include <string>

namespace dbc_parser {
namespace parser {
namespace {

TEST(EnvironmentVariableParserTest, ParsesBasicEnvironmentVariable) {
  const std::string input = "EV_ EngineSpeed 0 [0 8000] \"rpm\" 0 2364 DUMMY_NODE_VECTOR0 Vector__XXX;";
  
  auto result = EnvironmentVariableParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ("EngineSpeed", result->name);
  EXPECT_EQ(0, result->var_type);
  EXPECT_EQ(0.0, result->minimum);
  EXPECT_EQ(8000.0, result->maximum);
  EXPECT_EQ("rpm", result->unit);
  EXPECT_EQ(0, result->initial_value);
  EXPECT_EQ(2364, result->ev_id);
  EXPECT_EQ("DUMMY_NODE_VECTOR0", result->access_type);
  EXPECT_EQ("Vector__XXX", result->access_nodes);
}

TEST(EnvironmentVariableParserTest, ParsesWithDifferentVarType) {
  const std::string input = "EV_ EngineTemp 1 [-40 215] \"C\" 20 1243 DUMMY_NODE_VECTOR8 Vector__XXX;";
  
  auto result = EnvironmentVariableParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ("EngineTemp", result->name);
  EXPECT_EQ(1, result->var_type);
  EXPECT_EQ(-40.0, result->minimum);
  EXPECT_EQ(215.0, result->maximum);
  EXPECT_EQ("C", result->unit);
  EXPECT_EQ(20, result->initial_value);
  EXPECT_EQ(1243, result->ev_id);
  EXPECT_EQ("DUMMY_NODE_VECTOR8", result->access_type);
  EXPECT_EQ("Vector__XXX", result->access_nodes);
}

TEST(EnvironmentVariableParserTest, ParsesNegativeValues) {
  const std::string input = "EV_ Temperature 1 [-273.15 1000] \"K\" -10 5432 DUMMY_NODE_VECTOR8 Vector__XXX;";
  
  auto result = EnvironmentVariableParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ("Temperature", result->name);
  EXPECT_EQ(1, result->var_type);
  EXPECT_DOUBLE_EQ(-273.15, result->minimum);
  EXPECT_EQ(1000.0, result->maximum);
  EXPECT_EQ("K", result->unit);
  EXPECT_EQ(-10, result->initial_value);
  EXPECT_EQ(5432, result->ev_id);
  EXPECT_EQ("DUMMY_NODE_VECTOR8", result->access_type);
  EXPECT_EQ("Vector__XXX", result->access_nodes);
}

TEST(EnvironmentVariableParserTest, HandlesMultipleAccessNodes) {
  const std::string input = "EV_ SpeedLimit 0 [0 255] \"kph\" 120 7890 DUMMY_NODE_VECTOR0 Node1,Node2,Node3;";
  
  auto result = EnvironmentVariableParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ("SpeedLimit", result->name);
  EXPECT_EQ(0, result->var_type);
  EXPECT_EQ(0.0, result->minimum);
  EXPECT_EQ(255.0, result->maximum);
  EXPECT_EQ("kph", result->unit);
  EXPECT_EQ(120, result->initial_value);
  EXPECT_EQ(7890, result->ev_id);
  EXPECT_EQ("DUMMY_NODE_VECTOR0", result->access_type);
  EXPECT_EQ("Node1,Node2,Node3", result->access_nodes);
}

TEST(EnvironmentVariableParserTest, HandlesWhitespace) {
  const std::string input = "EV_   FuelLevel   0   [  0   100  ]   \"%\"   50   1234   DUMMY_NODE_VECTOR0   Node1 , Node2 ;";
  
  auto result = EnvironmentVariableParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ("FuelLevel", result->name);
  EXPECT_EQ(0, result->var_type);
  EXPECT_EQ(0.0, result->minimum);
  EXPECT_EQ(100.0, result->maximum);
  EXPECT_EQ("%", result->unit);
  EXPECT_EQ(50, result->initial_value);
  EXPECT_EQ(1234, result->ev_id);
  EXPECT_EQ("DUMMY_NODE_VECTOR0", result->access_type);
  EXPECT_EQ("Node1,Node2", result->access_nodes);
}

TEST(EnvironmentVariableParserTest, RejectsInvalidFormat) {
  // Missing EV_ keyword
  EXPECT_FALSE(EnvironmentVariableParser::Parse("EngineSpeed 0 [0 8000] \"rpm\" 0 2364 DUMMY_NODE_VECTOR0 Vector__XXX;").has_value());
  
  // Invalid var_type (non-numeric)
  EXPECT_FALSE(EnvironmentVariableParser::Parse("EV_ EngineSpeed X [0 8000] \"rpm\" 0 2364 DUMMY_NODE_VECTOR0 Vector__XXX;").has_value());
  
  // Missing bracket
  EXPECT_FALSE(EnvironmentVariableParser::Parse("EV_ EngineSpeed 0 0 8000] \"rpm\" 0 2364 DUMMY_NODE_VECTOR0 Vector__XXX;").has_value());
  
  // Invalid minimum (non-numeric)
  EXPECT_FALSE(EnvironmentVariableParser::Parse("EV_ EngineSpeed 0 [X 8000] \"rpm\" 0 2364 DUMMY_NODE_VECTOR0 Vector__XXX;").has_value());
  
  // Invalid maximum (non-numeric)
  EXPECT_FALSE(EnvironmentVariableParser::Parse("EV_ EngineSpeed 0 [0 X] \"rpm\" 0 2364 DUMMY_NODE_VECTOR0 Vector__XXX;").has_value());
  
  // Missing closing bracket
  EXPECT_FALSE(EnvironmentVariableParser::Parse("EV_ EngineSpeed 0 [0 8000 \"rpm\" 0 2364 DUMMY_NODE_VECTOR0 Vector__XXX;").has_value());
  
  // Invalid initial_value (non-numeric)
  EXPECT_FALSE(EnvironmentVariableParser::Parse("EV_ EngineSpeed 0 [0 8000] \"rpm\" X 2364 DUMMY_NODE_VECTOR0 Vector__XXX;").has_value());
  
  // Invalid ev_id (non-numeric)
  EXPECT_FALSE(EnvironmentVariableParser::Parse("EV_ EngineSpeed 0 [0 8000] \"rpm\" 0 X DUMMY_NODE_VECTOR0 Vector__XXX;").has_value());
  
  // Missing access_type
  EXPECT_FALSE(EnvironmentVariableParser::Parse("EV_ EngineSpeed 0 [0 8000] \"rpm\" 0 2364 Vector__XXX;").has_value());
  
  // Missing semicolon
  EXPECT_FALSE(EnvironmentVariableParser::Parse("EV_ EngineSpeed 0 [0 8000] \"rpm\" 0 2364 DUMMY_NODE_VECTOR0 Vector__XXX").has_value());
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 