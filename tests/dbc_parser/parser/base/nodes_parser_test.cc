#include "src/dbc_parser/parser/base/nodes_parser.h"

#include <string>
#include <vector>
#include "gtest/gtest.h"

namespace dbc_parser {
namespace parser {
namespace {

TEST(NodesParserTest, ParsesEmptyNodes) {
  const std::string input = "BU_:";
  auto result = NodesParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->empty());
}

TEST(NodesParserTest, ParsesSingleNode) {
  const std::string input = "BU_: ECU1";
  auto result = NodesParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->size(), 1);
  EXPECT_EQ((*result)[0].name, "ECU1");
}

TEST(NodesParserTest, ParsesMultipleNodes) {
  const std::string input = "BU_: ECU1 ECU2 Gateway Vector_XXX";
  auto result = NodesParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->size(), 4);
  EXPECT_EQ((*result)[0].name, "ECU1");
  EXPECT_EQ((*result)[1].name, "ECU2");
  EXPECT_EQ((*result)[2].name, "Gateway");
  EXPECT_EQ((*result)[3].name, "Vector_XXX");
}

TEST(NodesParserTest, HandlesWhitespace) {
  const std::string input = "  BU_  :  ECU1   ECU2    Gateway  ";
  auto result = NodesParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->size(), 3);
  EXPECT_EQ((*result)[0].name, "ECU1");
  EXPECT_EQ((*result)[1].name, "ECU2");
  EXPECT_EQ((*result)[2].name, "Gateway");
}

TEST(NodesParserTest, HandlesNodesWithSpecialChars) {
  const std::string input = "BU_: ECU_123 Node-With-Dash Node_With_Underscore";
  auto result = NodesParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->size(), 3);
  EXPECT_EQ((*result)[0].name, "ECU_123");
  EXPECT_EQ((*result)[1].name, "Node-With-Dash");
  EXPECT_EQ((*result)[2].name, "Node_With_Underscore");
}

TEST(NodesParserTest, RejectsInvalidFormat) {
  // Missing colon
  EXPECT_FALSE(NodesParser::Parse("BU_ ECU1 ECU2").has_value());
  
  // Missing BU_ keyword
  EXPECT_FALSE(NodesParser::Parse(": ECU1 ECU2").has_value());
  
  // Invalid keyword
  EXPECT_FALSE(NodesParser::Parse("BX_: ECU1 ECU2").has_value());
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 