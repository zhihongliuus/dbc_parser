#include "src/dbc_parser/parser/base/version_parser.h"

#include <string>
#include "gtest/gtest.h"

namespace dbc_parser {
namespace parser {
namespace {

TEST(VersionParserTest, ParsesValidVersionString) {
  const std::string input = "VERSION \"1.0\"";
  auto result = VersionParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->version, "1.0");
}

TEST(VersionParserTest, HandlesWhitespace) {
  const std::string input = "  VERSION   \"1.0\"  ";
  auto result = VersionParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->version, "1.0");
}

TEST(VersionParserTest, RejectsInvalidVersionFormat) {
  // Missing quotes
  EXPECT_FALSE(VersionParser::Parse("VERSION 1.0").has_value());
  
  // Missing VERSION keyword
  EXPECT_FALSE(VersionParser::Parse("\"1.0\"").has_value());
  
  // Empty version string
  EXPECT_FALSE(VersionParser::Parse("VERSION \"\"").has_value());
}

TEST(VersionParserTest, HandlesVersionWithSpecialCharacters) {
  const std::string input = "VERSION \"CANDB++ 1.0.123\"";
  auto result = VersionParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->version, "CANDB++ 1.0.123");
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 