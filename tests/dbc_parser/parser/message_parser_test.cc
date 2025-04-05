#include "src/dbc_parser/parser/message_parser.h"

#include <string>
#include <vector>
#include "gtest/gtest.h"

namespace dbc_parser {
namespace parser {
namespace {

TEST(MessageParserTest, ParsesBasicMessage) {
  const std::string input = "BO_ 123 EngineData: 8 Engine";
  auto result = MessageParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, 123);
  EXPECT_EQ(result->name, "EngineData");
  EXPECT_EQ(result->dlc, 8);
  EXPECT_EQ(result->sender, "Engine");
  EXPECT_TRUE(result->signals.empty());
}

TEST(MessageParserTest, ParsesMessageWithSignals) {
  const std::string input = 
      "BO_ 123 EngineData: 8 Engine\n"
      " SG_ RPM : 0|16@1+ (1,0) [0|8000] \"rpm\" Vector_XXX\n"
      " SG_ Temperature : 16|8@1+ (0.1,0) [0|120] \"degC\" Engine";
  auto result = MessageParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, 123);
  EXPECT_EQ(result->name, "EngineData");
  EXPECT_EQ(result->dlc, 8);
  EXPECT_EQ(result->sender, "Engine");
  
  // Check signals
  ASSERT_EQ(result->signals.size(), 2);
  
  // First signal: RPM
  const auto& rpm = result->signals[0];
  EXPECT_EQ(rpm.name, "RPM");
  EXPECT_EQ(rpm.start_bit, 0);
  EXPECT_EQ(rpm.length, 16);
  EXPECT_EQ(rpm.byte_order, 1);  // Intel format (little-endian)
  EXPECT_EQ(rpm.sign, SignType::kUnsigned);
  EXPECT_FLOAT_EQ(rpm.factor, 1.0);
  EXPECT_FLOAT_EQ(rpm.offset, 0.0);
  EXPECT_FLOAT_EQ(rpm.minimum, 0.0);
  EXPECT_FLOAT_EQ(rpm.maximum, 8000.0);
  EXPECT_EQ(rpm.unit, "rpm");
  
  std::vector<std::string> expected_receivers = {"Vector_XXX"};
  EXPECT_EQ(rpm.receivers.size(), expected_receivers.size());
  for (size_t i = 0; i < expected_receivers.size(); ++i) {
    EXPECT_EQ(rpm.receivers[i], expected_receivers[i]);
  }
  
  // Second signal: Temperature
  const auto& temp = result->signals[1];
  EXPECT_EQ(temp.name, "Temperature");
  EXPECT_EQ(temp.start_bit, 16);
  EXPECT_EQ(temp.length, 8);
  EXPECT_EQ(temp.byte_order, 1);
  EXPECT_EQ(temp.sign, SignType::kUnsigned);
  EXPECT_FLOAT_EQ(temp.factor, 0.1);
  EXPECT_FLOAT_EQ(temp.offset, 0.0);
  EXPECT_FLOAT_EQ(temp.minimum, 0.0);
  EXPECT_FLOAT_EQ(temp.maximum, 120.0);
  EXPECT_EQ(temp.unit, "degC");
  
  std::vector<std::string> expected_temp_receivers = {"Engine"};
  EXPECT_EQ(temp.receivers.size(), expected_temp_receivers.size());
  for (size_t i = 0; i < expected_temp_receivers.size(); ++i) {
    EXPECT_EQ(temp.receivers[i], expected_temp_receivers[i]);
  }
}

TEST(MessageParserTest, ParsesMessageWithMultiplexedSignals) {
  const std::string input = 
      "BO_ 123 EngineData: 8 Engine\n"
      " SG_ Mode M : 0|2@1+ (1,0) [0|3] \"\" Vector_XXX\n"
      " SG_ Temperature m0 : 8|8@1+ (0.1,0) [0|120] \"degC\" Engine\n"
      " SG_ RPM m1 : 8|16@1+ (1,0) [0|8000] \"rpm\" Engine";
  auto result = MessageParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, 123);
  
  // Check signals
  ASSERT_EQ(result->signals.size(), 3);
  
  // Multiplexor signal
  const auto& mode = result->signals[0];
  EXPECT_EQ(mode.name, "Mode");
  EXPECT_EQ(mode.multiplex_type, MultiplexType::kMultiplexor);
  EXPECT_FALSE(mode.multiplex_value.has_value());  // Not applicable for multiplexor
  
  // First multiplexed signal (m0)
  const auto& temp = result->signals[1];
  EXPECT_EQ(temp.name, "Temperature");
  EXPECT_EQ(temp.multiplex_type, MultiplexType::kMultiplexed);
  EXPECT_TRUE(temp.multiplex_value.has_value());
  EXPECT_EQ(temp.multiplex_value.value(), 0);
  
  // Second multiplexed signal (m1)
  const auto& rpm = result->signals[2];
  EXPECT_EQ(rpm.name, "RPM");
  EXPECT_EQ(rpm.multiplex_type, MultiplexType::kMultiplexed);
  EXPECT_TRUE(rpm.multiplex_value.has_value());
  EXPECT_EQ(rpm.multiplex_value.value(), 1);
}

TEST(MessageParserTest, HandlesSignedSignals) {
  const std::string input = 
      "BO_ 123 EngineData: 8 Engine\n"
      " SG_ Temperature : 0|8@1- (0.1,-40) [-40|80] \"degC\" Vector_XXX";
  auto result = MessageParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->signals.size(), 1);
  
  const auto& temp = result->signals[0];
  EXPECT_EQ(temp.sign, SignType::kSigned);
  EXPECT_FLOAT_EQ(temp.factor, 0.1);
  EXPECT_FLOAT_EQ(temp.offset, -40.0);
  EXPECT_FLOAT_EQ(temp.minimum, -40.0);
  EXPECT_FLOAT_EQ(temp.maximum, 80.0);
}

TEST(MessageParserTest, HandlesMotorolaFormat) {
  const std::string input = 
      "BO_ 123 EngineData: 8 Engine\n"
      " SG_ Temperature : 0|8@0+ (0.1,0) [0|120] \"degC\" Vector_XXX";
  auto result = MessageParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->signals.size(), 1);
  
  const auto& temp = result->signals[0];
  EXPECT_EQ(temp.byte_order, 0);  // Motorola format (big-endian)
}

TEST(MessageParserTest, HandlesMultipleReceivers) {
  const std::string input = 
      "BO_ 123 EngineData: 8 Engine\n"
      " SG_ Temperature : 0|8@1+ (0.1,0) [0|120] \"degC\" ECU1,ECU2,Gateway";
  auto result = MessageParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->signals.size(), 1);
  
  const auto& temp = result->signals[0];
  std::vector<std::string> expected_receivers = {"ECU1", "ECU2", "Gateway"};
  ASSERT_EQ(temp.receivers.size(), expected_receivers.size());
  for (size_t i = 0; i < expected_receivers.size(); ++i) {
    EXPECT_EQ(temp.receivers[i], expected_receivers[i]);
  }
}

TEST(MessageParserTest, HandlesWhitespace) {
  const std::string input = "  BO_  123  EngineData  :  8  Engine  ";
  auto result = MessageParser::Parse(input);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, 123);
  EXPECT_EQ(result->name, "EngineData");
  EXPECT_EQ(result->dlc, 8);
  EXPECT_EQ(result->sender, "Engine");
}

TEST(MessageParserTest, RejectsInvalidFormat) {
  // Missing colon
  EXPECT_FALSE(MessageParser::Parse("BO_ 123 EngineData 8 Engine").has_value());
  
  // Missing BO_ keyword
  EXPECT_FALSE(MessageParser::Parse("123 EngineData: 8 Engine").has_value());
  
  // Invalid ID (not a number)
  EXPECT_FALSE(MessageParser::Parse("BO_ ABC EngineData: 8 Engine").has_value());
  
  // Invalid DLC (not a number)
  EXPECT_FALSE(MessageParser::Parse("BO_ 123 EngineData: X Engine").has_value());
  
  // Missing sender
  EXPECT_FALSE(MessageParser::Parse("BO_ 123 EngineData: 8").has_value());
  
  // Invalid signal format
  EXPECT_FALSE(MessageParser::Parse(
      "BO_ 123 EngineData: 8 Engine\n"
      " SG_ RPM  0|16@1+ (1,0) [0|8000] \"rpm\" Vector_XXX").has_value());
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 