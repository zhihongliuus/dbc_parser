#include "src/dbc_parser/parser/attribute/attribute_definition_parser.h"

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dbc_parser {
namespace parser {
namespace {

using ::testing::Eq;
using ::testing::ElementsAre;
using ::testing::Optional;

class AttributeDefinitionParserTest : public ::testing::Test {};

TEST_F(AttributeDefinitionParserTest, ParsesIntegerAttribute) {
  const std::string kInput = "BA_DEF_ \"IntAttribute\" INT 0 100;";
  
  auto result = AttributeDefinitionParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "IntAttribute");
  EXPECT_EQ(result->object_type, AttributeObjectType::UNDEFINED);
  EXPECT_EQ(result->value_type, AttributeValueType::INT);
  EXPECT_EQ(result->min_value, 0);
  EXPECT_EQ(result->max_value, 100);
}

TEST_F(AttributeDefinitionParserTest, ParsesSignalAttribute) {
  const std::string kInput = "BA_DEF_ SG_ \"SignalAttribute\" FLOAT -10.5 10.5;";
  
  auto result = AttributeDefinitionParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "SignalAttribute");
  EXPECT_EQ(result->object_type, AttributeObjectType::SIGNAL);
  EXPECT_EQ(result->value_type, AttributeValueType::FLOAT);
  EXPECT_EQ(result->min_value, -10.5);
  EXPECT_EQ(result->max_value, 10.5);
}

TEST_F(AttributeDefinitionParserTest, ParsesMessageAttribute) {
  const std::string kInput = "BA_DEF_ BO_ \"MessageAttribute\" STRING;";
  
  auto result = AttributeDefinitionParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "MessageAttribute");
  EXPECT_EQ(result->object_type, AttributeObjectType::MESSAGE);
  EXPECT_EQ(result->value_type, AttributeValueType::STRING);
  EXPECT_FALSE(result->min_value.has_value());
  EXPECT_FALSE(result->max_value.has_value());
}

TEST_F(AttributeDefinitionParserTest, ParsesNodeAttribute) {
  const std::string kInput = "BA_DEF_ BU_ \"NodeAttribute\" HEX 0 255;";
  
  auto result = AttributeDefinitionParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "NodeAttribute");
  EXPECT_EQ(result->object_type, AttributeObjectType::NODE);
  EXPECT_EQ(result->value_type, AttributeValueType::HEX);
  EXPECT_EQ(result->min_value, 0);
  EXPECT_EQ(result->max_value, 255);
}

TEST_F(AttributeDefinitionParserTest, ParsesEnvironmentVariableAttribute) {
  const std::string kInput = "BA_DEF_ EV_ \"EnvVarAttribute\" INT 0 65535;";
  
  auto result = AttributeDefinitionParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "EnvVarAttribute");
  EXPECT_EQ(result->object_type, AttributeObjectType::ENV_VAR);
  EXPECT_EQ(result->value_type, AttributeValueType::INT);
  EXPECT_EQ(result->min_value, 0);
  EXPECT_EQ(result->max_value, 65535);
}

TEST_F(AttributeDefinitionParserTest, ParsesEnumAttribute) {
  const std::string kInput = "BA_DEF_ \"EnumAttribute\" ENUM \"Value1\",\"Value2\",\"Value3\";";
  
  auto result = AttributeDefinitionParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "EnumAttribute");
  EXPECT_EQ(result->object_type, AttributeObjectType::UNDEFINED);
  EXPECT_EQ(result->value_type, AttributeValueType::ENUM);
  EXPECT_FALSE(result->min_value.has_value());
  EXPECT_FALSE(result->max_value.has_value());
  EXPECT_THAT(result->enum_values, ElementsAre("Value1", "Value2", "Value3"));
}

TEST_F(AttributeDefinitionParserTest, HandlesWhitespace) {
  const std::string kInput = "BA_DEF_  BO_  \"MessageAttribute\"  STRING  ;";
  
  auto result = AttributeDefinitionParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "MessageAttribute");
  EXPECT_EQ(result->object_type, AttributeObjectType::MESSAGE);
  EXPECT_EQ(result->value_type, AttributeValueType::STRING);
}

TEST_F(AttributeDefinitionParserTest, RejectsInvalidFormat) {
  const std::vector<std::string> kInvalidInputs = {
    // Missing BA_DEF_ prefix
    "\"AttributeName\" INT 0 100;",
    // Missing semicolon
    "BA_DEF_ \"AttributeName\" INT 0 100",
    // Invalid object type
    "BA_DEF_ INVALID \"AttributeName\" INT 0 100;",
    // Invalid value type
    "BA_DEF_ \"AttributeName\" UNKNOWN 0 100;",
    // Missing attribute name
    "BA_DEF_ INT 0 100;",
    // Non-numeric min/max for numeric type
    "BA_DEF_ \"AttributeName\" INT abc 100;",
    // Empty input
    ""
  };
  
  for (const auto& input : kInvalidInputs) {
    bool has_value = AttributeDefinitionParser::Parse(input).has_value();
    EXPECT_FALSE(has_value) << "Input should be rejected: " << input;
  }
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 