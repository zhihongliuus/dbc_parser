#include "src/dbc_parser/parser/base/bit_timing_parser.h"

#include <string>
#include "gtest/gtest.h"

namespace dbc_parser {
namespace parser {
namespace {

TEST(BitTimingParserTest, ParsesValidBitTiming) {
  const std::string input = "BS_: 1000 62.5";
  auto result = BitTimingParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->baudrate, 1000);
  EXPECT_FLOAT_EQ(result->btr1_btr2, 62.5);
}

TEST(BitTimingParserTest, HandlesWhitespace) {
  const std::string input = "  BS_:   1000   62.5  ";
  auto result = BitTimingParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->baudrate, 1000);
  EXPECT_FLOAT_EQ(result->btr1_btr2, 62.5);
}

TEST(BitTimingParserTest, ParsesZeroValues) {
  const std::string input = "BS_: 0 0.0";
  auto result = BitTimingParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->baudrate, 0);
  EXPECT_FLOAT_EQ(result->btr1_btr2, 0.0);
}

TEST(BitTimingParserTest, RejectsInvalidFormat) {
  // Missing colon
  EXPECT_FALSE(BitTimingParser::Parse("BS_ 1000 62.5").has_value());
  
  // Missing BS_ keyword
  EXPECT_FALSE(BitTimingParser::Parse(": 1000 62.5").has_value());
  
  // Invalid keyword
  EXPECT_FALSE(BitTimingParser::Parse("BX_: 1000 62.5").has_value());
  
  // Missing baudrate
  EXPECT_FALSE(BitTimingParser::Parse("BS_: 62.5").has_value());
  
  // Missing BTR1/BTR2
  EXPECT_FALSE(BitTimingParser::Parse("BS_: 1000").has_value());
  
  // Non-numeric values
  EXPECT_FALSE(BitTimingParser::Parse("BS_: abc xyz").has_value());
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 