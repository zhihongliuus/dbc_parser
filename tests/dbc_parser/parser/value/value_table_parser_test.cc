#include "src/dbc_parser/parser/value/value_table_parser.h"

#include <string>
#include <unordered_map>
#include "gtest/gtest.h"

namespace dbc_parser {
namespace parser {
namespace {

TEST(ValueTableParserTest, ParsesEmptyValueTable) {
  const std::string input = "VAL_TABLE_ Engine_Status ;";
  auto result = ValueTableParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->name, "Engine_Status");
  EXPECT_TRUE(result->values.empty());
}

TEST(ValueTableParserTest, ParsesSingleValue) {
  const std::string input = "VAL_TABLE_ Engine_Status 0 \"Off\" ;";
  auto result = ValueTableParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->name, "Engine_Status");
  ASSERT_EQ(result->values.size(), 1);
  EXPECT_EQ(result->values.at(0), "Off");
}

TEST(ValueTableParserTest, ParsesMultipleValues) {
  const std::string input = "VAL_TABLE_ Engine_Status 0 \"Off\" 1 \"On\" 2 \"Error\" ;";
  auto result = ValueTableParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->name, "Engine_Status");
  ASSERT_EQ(result->values.size(), 3);
  EXPECT_EQ(result->values.at(0), "Off");
  EXPECT_EQ(result->values.at(1), "On");
  EXPECT_EQ(result->values.at(2), "Error");
}

TEST(ValueTableParserTest, HandlesWhitespace) {
  const std::string input = "  VAL_TABLE_  Engine_Status  0  \"Off\"   1  \"On\"  ;  ";
  auto result = ValueTableParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->name, "Engine_Status");
  ASSERT_EQ(result->values.size(), 2);
  EXPECT_EQ(result->values.at(0), "Off");
  EXPECT_EQ(result->values.at(1), "On");
}

TEST(ValueTableParserTest, HandlesSpecialCharactersInStrings) {
  const std::string input = "VAL_TABLE_ Engine_Status 0 \"Off - Standby\" 1 \"On & Running\" ;";
  auto result = ValueTableParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->name, "Engine_Status");
  ASSERT_EQ(result->values.size(), 2);
  EXPECT_EQ(result->values.at(0), "Off - Standby");
  EXPECT_EQ(result->values.at(1), "On & Running");
}

TEST(ValueTableParserTest, RejectsInvalidFormat) {
  // Missing semicolon
  EXPECT_FALSE(ValueTableParser::Parse("VAL_TABLE_ Engine_Status 0 \"Off\"").has_value());
  
  // Missing VAL_TABLE_ keyword
  EXPECT_FALSE(ValueTableParser::Parse("Engine_Status 0 \"Off\" ;").has_value());
  
  // Invalid keyword
  EXPECT_FALSE(ValueTableParser::Parse("VAL_TBLE_ Engine_Status 0 \"Off\" ;").has_value());
  
  // Missing table name
  EXPECT_FALSE(ValueTableParser::Parse("VAL_TABLE_ 0 \"Off\" ;").has_value());
  
  // No matching quotes
  EXPECT_FALSE(ValueTableParser::Parse("VAL_TABLE_ Engine_Status 0 \"Off ;").has_value());
  
  // Value without description
  EXPECT_FALSE(ValueTableParser::Parse("VAL_TABLE_ Engine_Status 0 ;").has_value());
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 