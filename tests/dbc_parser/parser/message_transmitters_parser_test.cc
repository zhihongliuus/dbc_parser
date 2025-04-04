#include "src/dbc_parser/parser/message_transmitters_parser.h"

#include <string>
#include <vector>
#include "gtest/gtest.h"

namespace dbc_parser {
namespace parser {
namespace {

TEST(MessageTransmittersParserTest, ParsesSingleTransmitter) {
  const std::string input = "BO_TX_BU_ 123 : Engine";
  auto result = MessageTransmittersParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->message_id, 123);
  ASSERT_EQ(result->transmitters.size(), 1);
  EXPECT_EQ(result->transmitters[0], "Engine");
}

TEST(MessageTransmittersParserTest, ParsesMultipleTransmitters) {
  const std::string input = "BO_TX_BU_ 123 : Engine,ECU1,Gateway";
  auto result = MessageTransmittersParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->message_id, 123);
  ASSERT_EQ(result->transmitters.size(), 3);
  EXPECT_EQ(result->transmitters[0], "Engine");
  EXPECT_EQ(result->transmitters[1], "ECU1");
  EXPECT_EQ(result->transmitters[2], "Gateway");
}

TEST(MessageTransmittersParserTest, HandlesWhitespace) {
  const std::string input = "  BO_TX_BU_  123  :  Engine  ,  ECU1  ,  Gateway  ";
  auto result = MessageTransmittersParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->message_id, 123);
  ASSERT_EQ(result->transmitters.size(), 3);
  EXPECT_EQ(result->transmitters[0], "Engine");
  EXPECT_EQ(result->transmitters[1], "ECU1");
  EXPECT_EQ(result->transmitters[2], "Gateway");
}

TEST(MessageTransmittersParserTest, HandlesNodesWithSpecialChars) {
  const std::string input = "BO_TX_BU_ 123 : ECU_123,Node-With-Dash,Node_With_Underscore";
  auto result = MessageTransmittersParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->message_id, 123);
  ASSERT_EQ(result->transmitters.size(), 3);
  EXPECT_EQ(result->transmitters[0], "ECU_123");
  EXPECT_EQ(result->transmitters[1], "Node-With-Dash");
  EXPECT_EQ(result->transmitters[2], "Node_With_Underscore");
}

TEST(MessageTransmittersParserTest, HandlesEmptyTransmittersList) {
  const std::string input = "BO_TX_BU_ 123 : ";
  auto result = MessageTransmittersParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->message_id, 123);
  EXPECT_TRUE(result->transmitters.empty());
}

TEST(MessageTransmittersParserTest, RejectsInvalidFormat) {
  // Missing colon
  EXPECT_FALSE(MessageTransmittersParser::Parse("BO_TX_BU_ 123 Engine").has_value());
  
  // Missing BO_TX_BU_ keyword
  EXPECT_FALSE(MessageTransmittersParser::Parse("123 : Engine").has_value());
  
  // Invalid keyword
  EXPECT_FALSE(MessageTransmittersParser::Parse("BO_TRX_BU_ 123 : Engine").has_value());
  
  // Invalid message ID (not a number)
  EXPECT_FALSE(MessageTransmittersParser::Parse("BO_TX_BU_ ABC : Engine").has_value());
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 