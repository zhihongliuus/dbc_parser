#include "src/dbc_parser/parser/comment_parser.h"

#include <string>
#include <utility>
#include <variant>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dbc_parser {
namespace parser {
namespace {

using ::testing::Eq;

class CommentParserTest : public ::testing::Test {};

TEST_F(CommentParserTest, ParsesNetworkComment) {
  const std::string kInput = "CM_ \"Network comment\";";
  
  auto result = CommentParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->type, CommentType::NETWORK);
  EXPECT_EQ(result->text, "Network comment");
  
  // Network comment doesn't have any identifier
  bool is_monostate = std::holds_alternative<std::monostate>(result->identifier);
  EXPECT_TRUE(is_monostate);
}

TEST_F(CommentParserTest, ParsesNodeComment) {
  const std::string kInput = "CM_ BU_ NodeName \"Node comment\";";
  
  auto result = CommentParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->type, CommentType::NODE);
  EXPECT_EQ(result->text, "Node comment");
  
  bool is_string = std::holds_alternative<std::string>(result->identifier);
  EXPECT_TRUE(is_string);
  EXPECT_EQ(std::get<std::string>(result->identifier), "NodeName");
}

TEST_F(CommentParserTest, ParsesMessageComment) {
  const std::string kInput = "CM_ BO_ 123 \"Message comment\";";
  
  auto result = CommentParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->type, CommentType::MESSAGE);
  EXPECT_EQ(result->text, "Message comment");
  
  bool is_int = std::holds_alternative<int>(result->identifier);
  EXPECT_TRUE(is_int);
  EXPECT_EQ(std::get<int>(result->identifier), 123);
}

TEST_F(CommentParserTest, ParsesSignalComment) {
  const std::string kInput = "CM_ SG_ 123 SignalName \"Signal comment\";";
  
  auto result = CommentParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->type, CommentType::SIGNAL);
  EXPECT_EQ(result->text, "Signal comment");
  
  using PairType = std::pair<int, std::string>;
  bool is_pair = std::holds_alternative<PairType>(result->identifier);
  EXPECT_TRUE(is_pair);
  const auto& id_pair = std::get<PairType>(result->identifier);
  EXPECT_EQ(id_pair.first, 123);
  EXPECT_EQ(id_pair.second, "SignalName");
}

TEST_F(CommentParserTest, ParsesEnvironmentVariableComment) {
  const std::string kInput = "CM_ EV_ EnvVarName \"Environment variable comment\";";
  
  auto result = CommentParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->type, CommentType::ENV_VAR);
  EXPECT_EQ(result->text, "Environment variable comment");
  
  bool is_string = std::holds_alternative<std::string>(result->identifier);
  EXPECT_TRUE(is_string);
  EXPECT_EQ(std::get<std::string>(result->identifier), "EnvVarName");
}

TEST_F(CommentParserTest, HandlesMultilineComment) {
  const std::string kInput = "CM_ \"This is a multiline\ncomment\nwith three lines\";";
  
  auto result = CommentParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->type, CommentType::NETWORK);
  EXPECT_EQ(result->text, "This is a multiline\ncomment\nwith three lines");
}

TEST_F(CommentParserTest, HandlesEscapedQuotes) {
  const std::string kInput = "CM_ \"Comment with \\\"quoted\\\" text\";";
  
  auto result = CommentParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->type, CommentType::NETWORK);
  EXPECT_EQ(result->text, "Comment with \"quoted\" text");
}

TEST_F(CommentParserTest, HandlesWhitespace) {
  const std::string kInput = "CM_   BU_    NodeName    \"  Node comment with spaces  \"  ;";
  
  auto result = CommentParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->type, CommentType::NODE);
  EXPECT_EQ(result->text, "  Node comment with spaces  ");
  
  bool is_string = std::holds_alternative<std::string>(result->identifier);
  EXPECT_TRUE(is_string);
  EXPECT_EQ(std::get<std::string>(result->identifier), "NodeName");
}

TEST_F(CommentParserTest, RejectsInvalidFormat) {
  const std::vector<std::string> kInvalidInputs = {
    // Missing CM_ prefix
    "\"Network comment\";",
    // Missing semicolon
    "CM_ \"Network comment\"",
    // Missing quotes around comment
    "CM_ Network comment;",
    // Invalid object type
    "CM_ XX_ NodeName \"Invalid type\";",
    // Signal comment with missing signal name
    "CM_ SG_ 123 \"Signal comment\";",
    // Empty comment
    "CM_ \"\";",
    // Empty input
    ""
  };
  
  for (const auto& input : kInvalidInputs) {
    EXPECT_FALSE(CommentParser::Parse(input).has_value()) 
      << "Input should be rejected: " << input;
  }
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 