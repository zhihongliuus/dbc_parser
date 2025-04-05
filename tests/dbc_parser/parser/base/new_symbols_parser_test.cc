#include "src/dbc_parser/parser/base/new_symbols_parser.h"

#include <string>
#include <vector>
#include "gtest/gtest.h"

namespace dbc_parser {
namespace parser {
namespace {

TEST(NewSymbolsParserTest, ParsesEmptySymbols) {
  const std::string input = "NS_ :";
  auto result = NewSymbolsParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->symbols.empty());
}

TEST(NewSymbolsParserTest, ParsesSingleSymbol) {
  const std::string input = "NS_ : CM_";
  auto result = NewSymbolsParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->symbols.size(), 1);
  EXPECT_EQ(result->symbols[0], "CM_");
}

TEST(NewSymbolsParserTest, ParsesMultipleSymbols) {
  const std::string input = "NS_ : CM_ BA_ VAL_ BO_ SG_";
  auto result = NewSymbolsParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->symbols.size(), 5);
  EXPECT_EQ(result->symbols[0], "CM_");
  EXPECT_EQ(result->symbols[1], "BA_");
  EXPECT_EQ(result->symbols[2], "VAL_");
  EXPECT_EQ(result->symbols[3], "BO_");
  EXPECT_EQ(result->symbols[4], "SG_");
}

TEST(NewSymbolsParserTest, HandlesWhitespace) {
  const std::string input = "  NS_   :  CM_  BA_   VAL_  ";
  auto result = NewSymbolsParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->symbols.size(), 3);
  EXPECT_EQ(result->symbols[0], "CM_");
  EXPECT_EQ(result->symbols[1], "BA_");
  EXPECT_EQ(result->symbols[2], "VAL_");
}

TEST(NewSymbolsParserTest, RejectsInvalidFormat) {
  // Missing colon
  EXPECT_FALSE(NewSymbolsParser::Parse("NS_ CM_ BA_").has_value());
  
  // Missing NS_ keyword
  EXPECT_FALSE(NewSymbolsParser::Parse(": CM_ BA_").has_value());
  
  // Invalid keyword
  EXPECT_FALSE(NewSymbolsParser::Parse("NX_ : CM_ BA_").has_value());
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 