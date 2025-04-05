#include "src/dbc_parser/parser/message/message_transmitters_parser.h"

#include <string>
#include <string_view>

#include "gtest/gtest.h"

namespace dbc_parser {
namespace parser {
namespace {

TEST(MessageTransmittersParserTest, ParsesBasicTransmitters) {
  const std::string kInput = "BO_TX_BU_ 123 : Node1, Node2;";
  
  auto result = MessageTransmittersParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(123, result->message_id);
  ASSERT_EQ(2, result->transmitters.size());
  EXPECT_EQ("Node1", result->transmitters[0]);
  EXPECT_EQ("Node2", result->transmitters[1]);
}

TEST(MessageTransmittersParserTest, HandlesMissingSemicolon) {
  const std::string kInput = "BO_TX_BU_ 123 : Node1, Node2";
  
  auto result = MessageTransmittersParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(123, result->message_id);
  ASSERT_EQ(2, result->transmitters.size());
  EXPECT_EQ("Node1", result->transmitters[0]);
  EXPECT_EQ("Node2", result->transmitters[1]);
}

TEST(MessageTransmittersParserTest, HandlesSingleTransmitter) {
  const std::string kInput = "BO_TX_BU_ 123 : Node1;";
  
  auto result = MessageTransmittersParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(123, result->message_id);
  ASSERT_EQ(1, result->transmitters.size());
  EXPECT_EQ("Node1", result->transmitters[0]);
}

TEST(MessageTransmittersParserTest, HandlesInvalidInput) {
  const std::string kInput = "WRONG_KEY 123 : Node1;";
  
  auto result = MessageTransmittersParser::Parse(kInput);
  EXPECT_FALSE(result.has_value());
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 