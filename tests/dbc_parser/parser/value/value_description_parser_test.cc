#include "src/dbc_parser/parser/value/value_description_parser.h"

#include <map>
#include <string>
#include <utility>
#include <variant>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dbc_parser {
namespace parser {
namespace {

using ::testing::Eq;
using ::testing::Pair;
using ::testing::UnorderedElementsAre;

class ValueDescriptionParserTest : public ::testing::Test {};

TEST_F(ValueDescriptionParserTest, ParsesSignalValueDescription) {
  const std::string kInput = "VAL_ 123 SignalName 0 \"Off\" 1 \"On\" 2 \"Error\";";
  
  auto result = ValueDescriptionParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->type, ValueDescriptionType::SIGNAL);
  
  // Check identifier
  bool is_signal_id = std::holds_alternative<std::pair<int, std::string>>(result->identifier);
  ASSERT_TRUE(is_signal_id);
  const auto& id_pair = std::get<std::pair<int, std::string>>(result->identifier);
  EXPECT_EQ(id_pair.first, 123);
  EXPECT_EQ(id_pair.second, "SignalName");
  
  // Check value descriptions
  EXPECT_THAT(result->value_descriptions, UnorderedElementsAre(
    Pair(0, "Off"),
    Pair(1, "On"),
    Pair(2, "Error")
  ));
}

TEST_F(ValueDescriptionParserTest, ParsesEnvironmentVariableValueDescription) {
  const std::string kInput = "VAL_ EnvVarName 0 \"Inactive\" 1 \"Active\";";
  
  auto result = ValueDescriptionParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->type, ValueDescriptionType::ENV_VAR);
  
  // Check identifier
  bool is_env_var = std::holds_alternative<std::string>(result->identifier);
  ASSERT_TRUE(is_env_var);
  EXPECT_EQ(std::get<std::string>(result->identifier), "EnvVarName");
  
  // Check value descriptions
  EXPECT_THAT(result->value_descriptions, UnorderedElementsAre(
    Pair(0, "Inactive"),
    Pair(1, "Active")
  ));
}

TEST_F(ValueDescriptionParserTest, HandlesSingleValueDescription) {
  const std::string kInput = "VAL_ 123 SignalName 1 \"Active\";";
  
  auto result = ValueDescriptionParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->type, ValueDescriptionType::SIGNAL);
  EXPECT_THAT(result->value_descriptions, UnorderedElementsAre(
    Pair(1, "Active")
  ));
}

TEST_F(ValueDescriptionParserTest, HandlesNegativeValues) {
  const std::string kInput = "VAL_ 123 SignalName -1 \"Error\" 0 \"Off\" 1 \"On\";";
  
  auto result = ValueDescriptionParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->type, ValueDescriptionType::SIGNAL);
  EXPECT_THAT(result->value_descriptions, UnorderedElementsAre(
    Pair(-1, "Error"),
    Pair(0, "Off"),
    Pair(1, "On")
  ));
}

TEST_F(ValueDescriptionParserTest, HandlesWhitespace) {
  const std::string kInput = "VAL_  123  SignalName  0  \"Off\"  1  \"On\" ;";
  
  auto result = ValueDescriptionParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->type, ValueDescriptionType::SIGNAL);
  EXPECT_THAT(result->value_descriptions, UnorderedElementsAre(
    Pair(0, "Off"),
    Pair(1, "On")
  ));
}

TEST_F(ValueDescriptionParserTest, HandlesEscapedQuotes) {
  const std::string kInput = "VAL_ 123 SignalName 0 \"Contains \\\"quotes\\\"\" 1 \"Normal\";";
  
  auto result = ValueDescriptionParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->type, ValueDescriptionType::SIGNAL);
  EXPECT_THAT(result->value_descriptions, UnorderedElementsAre(
    Pair(0, "Contains \"quotes\""),
    Pair(1, "Normal")
  ));
}

TEST_F(ValueDescriptionParserTest, RejectsInvalidFormat) {
  const std::vector<std::string> kInvalidInputs = {
    // Missing VAL_ prefix
    "123 SignalName 0 \"Off\" 1 \"On\";",
    // Missing semicolon
    "VAL_ 123 SignalName 0 \"Off\" 1 \"On\"",
    // Incomplete value-description pair
    "VAL_ 123 SignalName 0 \"Off\" 1;",
    // Missing quotes around description
    "VAL_ 123 SignalName 0 Off 1 \"On\";",
    // Invalid value (non-integer)
    "VAL_ 123 SignalName A \"Error\" 0 \"Off\";",
    // Empty input
    ""
  };
  
  for (const auto& input : kInvalidInputs) {
    bool has_value = ValueDescriptionParser::Parse(input).has_value();
    EXPECT_FALSE(has_value) << "Input should be rejected: " << input;
  }
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 