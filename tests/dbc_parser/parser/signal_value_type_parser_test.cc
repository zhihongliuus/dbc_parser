#include "src/dbc_parser/parser/signal_value_type_parser.h"

#include <gtest/gtest.h>
#include <optional>
#include <string>

namespace dbc_parser {
namespace parser {
namespace {

TEST(SignalValueTypeParserTest, ParsesBasicSignalValueType) {
  const std::string input = "SIG_VALTYPE_ 123 EngineSpeed 1;";
  
  auto result = SignalValueTypeParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(123, result->message_id);
  EXPECT_EQ("EngineSpeed", result->signal_name);
  EXPECT_EQ(1, result->type);  // 1 = float
}

TEST(SignalValueTypeParserTest, ParsesNegativeMessageId) {
  const std::string input = "SIG_VALTYPE_ -42 Temperature 2;";
  
  auto result = SignalValueTypeParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(-42, result->message_id);
  EXPECT_EQ("Temperature", result->signal_name);
  EXPECT_EQ(2, result->type);  // 2 = double
}

TEST(SignalValueTypeParserTest, ParsesIntegerType) {
  const std::string input = "SIG_VALTYPE_ 1024 EngineRPM 0;";
  
  auto result = SignalValueTypeParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(1024, result->message_id);
  EXPECT_EQ("EngineRPM", result->signal_name);
  EXPECT_EQ(0, result->type);  // 0 = integer
}

TEST(SignalValueTypeParserTest, HandlesWhitespace) {
  const std::string input = "SIG_VALTYPE_   500    EngineTemp    1   ;";
  
  auto result = SignalValueTypeParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(500, result->message_id);
  EXPECT_EQ("EngineTemp", result->signal_name);
  EXPECT_EQ(1, result->type);
}

TEST(SignalValueTypeParserTest, HandlesCompoundIdentifiers) {
  const std::string input = "SIG_VALTYPE_ 100 Engine_Speed_1 0;";
  
  auto result = SignalValueTypeParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(100, result->message_id);
  EXPECT_EQ("Engine_Speed_1", result->signal_name);
  EXPECT_EQ(0, result->type);
}

TEST(SignalValueTypeParserTest, RejectsInvalidFormat) {
  // Missing SIG_VALTYPE_ keyword
  EXPECT_FALSE(SignalValueTypeParser::Parse("123 EngineSpeed 1;").has_value());
  
  // Missing message ID
  EXPECT_FALSE(SignalValueTypeParser::Parse("SIG_VALTYPE_ EngineSpeed 1;").has_value());
  
  // Missing signal name
  EXPECT_FALSE(SignalValueTypeParser::Parse("SIG_VALTYPE_ 123 1;").has_value());
  
  // Missing signal type
  EXPECT_FALSE(SignalValueTypeParser::Parse("SIG_VALTYPE_ 123 EngineSpeed;").has_value());
  
  // Invalid signal type (only 0, 1, 2 are valid)
  EXPECT_FALSE(SignalValueTypeParser::Parse("SIG_VALTYPE_ 123 EngineSpeed 3;").has_value());
  
  // Missing semicolon
  EXPECT_FALSE(SignalValueTypeParser::Parse("SIG_VALTYPE_ 123 EngineSpeed 1").has_value());
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 