#include "src/dbc_parser/parser/attribute/attribute_value_parser.h"

#include <optional>
#include <string>
#include <utility>
#include <variant>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dbc_parser {
namespace parser {
namespace {

using ::testing::Eq;

class AttributeValueParserTest : public ::testing::Test {};

TEST_F(AttributeValueParserTest, ParsesNetworkAttribute) {
  const std::string kInput = "BA_ \"NetworkAttr\" 42;";
  
  auto result = AttributeValueParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "NetworkAttr");
  EXPECT_EQ(result->object_type, AttributeObjectType::UNDEFINED);
  
  // Check if the object_id is a std::monostate (network-level attribute)
  bool is_monostate = std::holds_alternative<std::monostate>(result->object_id);
  EXPECT_TRUE(is_monostate);
  
  EXPECT_EQ(std::get<int>(result->value), 42);
}

TEST_F(AttributeValueParserTest, ParsesNodeAttribute) {
  const std::string kInput = "BA_ \"NodeAttr\" BU_ \"ECU1\" 42;";
  
  auto result = AttributeValueParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "NodeAttr");
  EXPECT_EQ(result->object_type, AttributeObjectType::NODE);
  
  // Check if the object_id is a string (node name)
  bool is_string = std::holds_alternative<std::string>(result->object_id);
  EXPECT_TRUE(is_string);
  
  EXPECT_EQ(std::get<std::string>(result->object_id), "ECU1");
  EXPECT_EQ(std::get<int>(result->value), 42);
}

TEST_F(AttributeValueParserTest, ParsesMessageAttribute) {
  const std::string kInput = "BA_ \"MessageAttr\" BO_ 123 42;";
  
  auto result = AttributeValueParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "MessageAttr");
  EXPECT_EQ(result->object_type, AttributeObjectType::MESSAGE);
  
  // Check if the object_id is an int (message ID)
  bool is_int = std::holds_alternative<int>(result->object_id);
  EXPECT_TRUE(is_int);
  
  EXPECT_EQ(std::get<int>(result->object_id), 123);
  EXPECT_EQ(std::get<int>(result->value), 42);
}

TEST_F(AttributeValueParserTest, ParsesSignalAttribute) {
  const std::string kInput = "BA_ \"SignalAttr\" SG_ 123 \"SignalName\" 42;";
  
  auto result = AttributeValueParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "SignalAttr");
  EXPECT_EQ(result->object_type, AttributeObjectType::SIGNAL);
  
  // Check if the object_id is a pair (message ID, signal name)
  bool is_pair = std::holds_alternative<std::pair<int, std::string>>(result->object_id);
  EXPECT_TRUE(is_pair);
  
  const auto& id_pair = std::get<std::pair<int, std::string>>(result->object_id);
  EXPECT_EQ(id_pair.first, 123);
  EXPECT_EQ(id_pair.second, "SignalName");
  EXPECT_EQ(std::get<int>(result->value), 42);
}

TEST_F(AttributeValueParserTest, ParsesEnvironmentVariableAttribute) {
  const std::string kInput = "BA_ \"EnvVarAttr\" EV_ \"EnvVar\" 42;";
  
  auto result = AttributeValueParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "EnvVarAttr");
  EXPECT_EQ(result->object_type, AttributeObjectType::ENV_VAR);
  
  // Check if the object_id is a string (env var name)
  bool is_string = std::holds_alternative<std::string>(result->object_id);
  EXPECT_TRUE(is_string);
  
  EXPECT_EQ(std::get<std::string>(result->object_id), "EnvVar");
  EXPECT_EQ(std::get<int>(result->value), 42);
}

TEST_F(AttributeValueParserTest, ParsesFloatValue) {
  const std::string kInput = "BA_ \"FloatAttr\" 3.14;";
  
  auto result = AttributeValueParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "FloatAttr");
  
  // Check if the value is a double
  bool is_double = std::holds_alternative<double>(result->value);
  EXPECT_TRUE(is_double);
  
  EXPECT_DOUBLE_EQ(std::get<double>(result->value), 3.14);
}

TEST_F(AttributeValueParserTest, ParsesStringValue) {
  const std::string kInput = "BA_ \"StringAttr\" \"String Value\";";
  
  auto result = AttributeValueParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "StringAttr");
  
  // Check if the value is a string
  bool is_string = std::holds_alternative<std::string>(result->value);
  EXPECT_TRUE(is_string);
  
  EXPECT_EQ(std::get<std::string>(result->value), "String Value");
}

TEST_F(AttributeValueParserTest, HandlesWhitespace) {
  const std::string kInput = "BA_  \"NetworkAttr\"  42  ;";
  
  auto result = AttributeValueParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "NetworkAttr");
  EXPECT_EQ(std::get<int>(result->value), 42);
}

TEST_F(AttributeValueParserTest, RejectsInvalidFormat) {
  const std::vector<std::string> kInvalidInputs = {
    // Missing BA_ prefix
    "\"AttributeName\" 42;",
    // Missing semicolon
    "BA_ \"AttributeName\" 42",
    // Invalid object type
    "BA_ \"AttributeName\" INVALID \"NodeName\" 42;",
    // Missing attribute name
    "BA_ BU_ \"NodeName\" 42;",
    // Missing object identifier
    "BA_ \"AttributeName\" BU_ 42;",
    // Missing value
    "BA_ \"AttributeName\" BU_ \"NodeName\";",
    // Empty input
    ""
  };
  
  for (const auto& input : kInvalidInputs) {
    bool has_value = AttributeValueParser::Parse(input).has_value();
    EXPECT_FALSE(has_value) << "Input should be rejected: " << input;
  }
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 