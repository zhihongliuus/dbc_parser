#include "src/dbc_parser/parser/message/signal_type_def_parser.h"

#include <gtest/gtest.h>
#include <optional>
#include <string>

namespace dbc_parser {
namespace parser {
namespace {

TEST(SignalTypeDefParserTest, ParsesBasicSignalTypeDef) {
  const std::string input = "SIG_TYPE_DEF_ SignalTypeFloat: 32, 1, +, 1, 0, 0, 100, \"m/s\", 0, VALUE_TABLE;";
  
  auto result = SignalTypeDefParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ("SignalTypeFloat", result->name);
  EXPECT_EQ(32, result->size);
  EXPECT_EQ(1, result->byte_order);
  EXPECT_EQ("+", result->value_type);
  EXPECT_DOUBLE_EQ(1.0, result->factor);
  EXPECT_DOUBLE_EQ(0.0, result->offset);
  EXPECT_DOUBLE_EQ(0.0, result->minimum);
  EXPECT_DOUBLE_EQ(100.0, result->maximum);
  EXPECT_EQ("m/s", result->unit);
  EXPECT_DOUBLE_EQ(0.0, result->default_value);
  EXPECT_EQ("VALUE_TABLE", result->value_table);
}

TEST(SignalTypeDefParserTest, ParsesSignedValueType) {
  const std::string input = "SIG_TYPE_DEF_ SignalTypeSigned: 16, 0, -, 0.1, -100, -200, 200, \"C\", 0, ;";
  
  auto result = SignalTypeDefParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ("SignalTypeSigned", result->name);
  EXPECT_EQ(16, result->size);
  EXPECT_EQ(0, result->byte_order);
  EXPECT_EQ("-", result->value_type);
  EXPECT_DOUBLE_EQ(0.1, result->factor);
  EXPECT_DOUBLE_EQ(-100.0, result->offset);
  EXPECT_DOUBLE_EQ(-200.0, result->minimum);
  EXPECT_DOUBLE_EQ(200.0, result->maximum);
  EXPECT_EQ("C", result->unit);
  EXPECT_DOUBLE_EQ(0.0, result->default_value);
  EXPECT_TRUE(result->value_table.empty());
}

TEST(SignalTypeDefParserTest, HandlesWhitespace) {
  const std::string input = "SIG_TYPE_DEF_   SignalType  :  8  ,  0  ,  +  ,  1  ,  0  ,  0  ,  255  ,  \"%\"  ,  0  ,  ;";
  
  auto result = SignalTypeDefParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ("SignalType", result->name);
  EXPECT_EQ(8, result->size);
  EXPECT_EQ(0, result->byte_order);
  EXPECT_EQ("+", result->value_type);
  EXPECT_DOUBLE_EQ(1.0, result->factor);
  EXPECT_DOUBLE_EQ(0.0, result->offset);
  EXPECT_DOUBLE_EQ(0.0, result->minimum);
  EXPECT_DOUBLE_EQ(255.0, result->maximum);
  EXPECT_EQ("%", result->unit);
  EXPECT_DOUBLE_EQ(0.0, result->default_value);
  EXPECT_TRUE(result->value_table.empty());
}

TEST(SignalTypeDefParserTest, HandlesFloatingPointValues) {
  const std::string input = "SIG_TYPE_DEF_ FloatSignal: 16, 1, +, 0.01, -10.5, -50.25, 100.75, \"V\", 0.33, ;";
  
  auto result = SignalTypeDefParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ("FloatSignal", result->name);
  EXPECT_EQ(16, result->size);
  EXPECT_EQ(1, result->byte_order);
  EXPECT_EQ("+", result->value_type);
  EXPECT_DOUBLE_EQ(0.01, result->factor);
  EXPECT_DOUBLE_EQ(-10.5, result->offset);
  EXPECT_DOUBLE_EQ(-50.25, result->minimum);
  EXPECT_DOUBLE_EQ(100.75, result->maximum);
  EXPECT_EQ("V", result->unit);
  EXPECT_DOUBLE_EQ(0.33, result->default_value);
  EXPECT_TRUE(result->value_table.empty());
}

TEST(SignalTypeDefParserTest, RejectsInvalidFormat) {
  // Missing SIG_TYPE_DEF_ keyword
  EXPECT_FALSE(SignalTypeDefParser::Parse("SignalType: 8, 0, +, 1, 0, 0, 255, \"%\", 0, ;").has_value());
  
  // Missing colon
  EXPECT_FALSE(SignalTypeDefParser::Parse("SIG_TYPE_DEF_ SignalType 8, 0, +, 1, 0, 0, 255, \"%\", 0, ;").has_value());
  
  // Missing comma
  EXPECT_FALSE(SignalTypeDefParser::Parse("SIG_TYPE_DEF_ SignalType: 8 0, +, 1, 0, 0, 255, \"%\", 0, ;").has_value());
  
  // Invalid size (non-numeric)
  EXPECT_FALSE(SignalTypeDefParser::Parse("SIG_TYPE_DEF_ SignalType: X, 0, +, 1, 0, 0, 255, \"%\", 0, ;").has_value());
  
  // Invalid byte order (non-numeric)
  EXPECT_FALSE(SignalTypeDefParser::Parse("SIG_TYPE_DEF_ SignalType: 8, X, +, 1, 0, 0, 255, \"%\", 0, ;").has_value());
  
  // Missing semicolon
  EXPECT_FALSE(SignalTypeDefParser::Parse("SIG_TYPE_DEF_ SignalType: 8, 0, +, 1, 0, 0, 255, \"%\", 0, ").has_value());
  
  // Too few parameters
  EXPECT_FALSE(SignalTypeDefParser::Parse("SIG_TYPE_DEF_ SignalType: 8, 0, +, 1, 0, 0, 255, ;").has_value());
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 