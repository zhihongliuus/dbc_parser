#include "src/dbc_parser/parser/attribute/attribute_definition_default_parser.h"

#include <optional>
#include <string>
#include <variant>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dbc_parser {
namespace parser {
namespace {

using ::testing::Eq;

class AttributeDefinitionDefaultParserTest : public ::testing::Test {};

TEST_F(AttributeDefinitionDefaultParserTest, ParsesIntegerDefault) {
  const std::string kInput = "BA_DEF_DEF_ \"IntAttribute\" 42;";
  
  auto result = AttributeDefinitionDefaultParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "IntAttribute");
  EXPECT_EQ(result->value_type, AttributeValueType::INT);
  EXPECT_EQ(std::get<int>(result->default_value), 42);
}

TEST_F(AttributeDefinitionDefaultParserTest, ParsesFloatDefault) {
  const std::string kInput = "BA_DEF_DEF_ \"FloatAttribute\" 3.14;";
  
  auto result = AttributeDefinitionDefaultParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "FloatAttribute");
  EXPECT_EQ(result->value_type, AttributeValueType::FLOAT);
  EXPECT_DOUBLE_EQ(std::get<double>(result->default_value), 3.14);
}

TEST_F(AttributeDefinitionDefaultParserTest, ParsesStringDefault) {
  const std::string kInput = "BA_DEF_DEF_ \"StringAttribute\" \"Default Value\";";
  
  auto result = AttributeDefinitionDefaultParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "StringAttribute");
  EXPECT_EQ(result->value_type, AttributeValueType::STRING);
  EXPECT_EQ(std::get<std::string>(result->default_value), "Default Value");
}

TEST_F(AttributeDefinitionDefaultParserTest, ParsesEnumDefault) {
  const std::string kInput = "BA_DEF_DEF_ \"EnumAttribute\" 2;";
  
  auto result = AttributeDefinitionDefaultParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "EnumAttribute");
  EXPECT_EQ(result->value_type, AttributeValueType::ENUM);
  EXPECT_EQ(std::get<int>(result->default_value), 2);
}

TEST_F(AttributeDefinitionDefaultParserTest, HandlesWhitespace) {
  const std::string kInput = "BA_DEF_DEF_  \"IntAttribute\"  42  ;";
  
  auto result = AttributeDefinitionDefaultParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "IntAttribute");
  EXPECT_EQ(std::get<int>(result->default_value), 42);
}

TEST_F(AttributeDefinitionDefaultParserTest, HandlesNegativeValues) {
  const std::string kInput = "BA_DEF_DEF_ \"IntAttribute\" -10;";
  
  auto result = AttributeDefinitionDefaultParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "IntAttribute");
  EXPECT_EQ(std::get<int>(result->default_value), -10);
}

TEST_F(AttributeDefinitionDefaultParserTest, RejectsInvalidFormat) {
  const std::vector<std::string> kInvalidInputs = {
    // Missing BA_DEF_DEF_ prefix
    "\"AttributeName\" 42;",
    // Missing semicolon
    "BA_DEF_DEF_ \"AttributeName\" 42",
    // Missing attribute name
    "BA_DEF_DEF_ 42;",
    // Missing default value
    "BA_DEF_DEF_ \"AttributeName\";",
    // Empty input
    ""
  };
  
  for (const auto& input : kInvalidInputs) {
    bool has_value = AttributeDefinitionDefaultParser::Parse(input).has_value();
    EXPECT_FALSE(has_value) << "Input should be rejected: " << input;
  }
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 