#include "src/dbc_parser/parser/environment/environment_variable_data_parser.h"

#include <gtest/gtest.h>
#include <optional>
#include <string>

namespace dbc_parser {
namespace parser {
namespace {

TEST(EnvironmentVariableDataParserTest, ParsesBasicEnvironmentVariableData) {
  const std::string input = "ENVVAR_DATA_ EngineSpeed: 4;";
  
  auto result = EnvironmentVariableDataParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ("EngineSpeed", result->name);
  EXPECT_EQ(input, result->data);
}

TEST(EnvironmentVariableDataParserTest, HandlesWhitespace) {
  const std::string input = "ENVVAR_DATA_   EngineTemp  :   8  ;";
  
  auto result = EnvironmentVariableDataParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ("EngineTemp", result->name);
  EXPECT_EQ(input, result->data);
}

TEST(EnvironmentVariableDataParserTest, HandlesNamesWithSpecialChars) {
  const std::string input = "ENVVAR_DATA_ Engine_Temp_1: 2;";
  
  auto result = EnvironmentVariableDataParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ("Engine_Temp_1", result->name);
  EXPECT_EQ(input, result->data);
}

TEST(EnvironmentVariableDataParserTest, RejectsInvalidFormat) {
  // Missing ENVVAR_DATA_ keyword
  EXPECT_FALSE(EnvironmentVariableDataParser::Parse("EngineSpeed: 4;").has_value());
  
  // Missing colon
  EXPECT_FALSE(EnvironmentVariableDataParser::Parse("ENVVAR_DATA_ EngineSpeed 4;").has_value());
  
  // Missing semicolon
  EXPECT_FALSE(EnvironmentVariableDataParser::Parse("ENVVAR_DATA_ EngineSpeed: 4").has_value());
  
  // Invalid data size (non-numeric)
  EXPECT_FALSE(EnvironmentVariableDataParser::Parse("ENVVAR_DATA_ EngineSpeed: X;").has_value());
  
  // Empty name
  EXPECT_FALSE(EnvironmentVariableDataParser::Parse("ENVVAR_DATA_ : 4;").has_value());
  
  // Missing data size
  EXPECT_FALSE(EnvironmentVariableDataParser::Parse("ENVVAR_DATA_ EngineSpeed:;").has_value());
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 