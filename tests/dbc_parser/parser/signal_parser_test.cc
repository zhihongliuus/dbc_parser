#include "src/dbc_parser/parser/signal_parser.h"

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dbc_parser {
namespace parser {
namespace {

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;

class SignalParserTest : public ::testing::Test {};

TEST_F(SignalParserTest, ParsesBasicSignal) {
  const std::string kInput = "SG_ SignalName : 8|16@1+ (0.1,0) [0|655.35] \"km/h\" ECU1,ECU2";
  
  auto result = SignalParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "SignalName");
  EXPECT_EQ(result->start_bit, 8);
  EXPECT_EQ(result->signal_size, 16);
  EXPECT_TRUE(result->is_little_endian);
  EXPECT_TRUE(result->is_signed);
  EXPECT_DOUBLE_EQ(result->factor, 0.1);
  EXPECT_DOUBLE_EQ(result->offset, 0.0);
  EXPECT_DOUBLE_EQ(result->minimum, 0.0);
  EXPECT_DOUBLE_EQ(result->maximum, 655.35);
  EXPECT_EQ(result->unit, "km/h");
  EXPECT_THAT(result->receivers, ElementsAre("ECU1", "ECU2"));
  EXPECT_FALSE(result->is_multiplexer);
  EXPECT_FALSE(result->multiplexed_by.has_value());
}

TEST_F(SignalParserTest, ParsesUnsignedSignal) {
  const std::string kInput = "SG_ EngineTemp : 16|8@1- (2.5,-40) [-40|250] \"C\" Vector__XXX";
  
  auto result = SignalParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "EngineTemp");
  EXPECT_EQ(result->start_bit, 16);
  EXPECT_EQ(result->signal_size, 8);
  EXPECT_TRUE(result->is_little_endian);
  EXPECT_FALSE(result->is_signed);
  EXPECT_DOUBLE_EQ(result->factor, 2.5);
  EXPECT_DOUBLE_EQ(result->offset, -40.0);
  EXPECT_DOUBLE_EQ(result->minimum, -40.0);
  EXPECT_DOUBLE_EQ(result->maximum, 250.0);
  EXPECT_EQ(result->unit, "C");
  EXPECT_THAT(result->receivers, ElementsAre("Vector__XXX"));
}

TEST_F(SignalParserTest, ParsesBigEndianSignal) {
  const std::string kInput = "SG_ EngineRPM : 24|16@0+ (1,0) [0|16000] \"rpm\" ECU1";
  
  auto result = SignalParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->start_bit, 24);
  EXPECT_EQ(result->signal_size, 16);
  EXPECT_FALSE(result->is_little_endian);
  EXPECT_TRUE(result->is_signed);
}

TEST_F(SignalParserTest, ParsesMultiplexerSignal) {
  const std::string kInput = "SG_ MuxSelector M : 0|4@1+ (1,0) [0|15] \"\" ECU1";
  
  auto result = SignalParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "MuxSelector");
  EXPECT_TRUE(result->is_multiplexer);
  EXPECT_FALSE(result->multiplexed_by.has_value());
}

TEST_F(SignalParserTest, ParsesMultiplexedSignal) {
  const std::string kInput = "SG_ Temperature m2 : 8|16@1+ (0.1,-40) [-40|150] \"C\" ECU1";
  
  auto result = SignalParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "Temperature");
  EXPECT_FALSE(result->is_multiplexer);
  EXPECT_TRUE(result->multiplexed_by.has_value());
  EXPECT_EQ(result->multiplexed_by.value(), 2);
}

TEST_F(SignalParserTest, HandlesWhitespace) {
  const std::string kInput = "SG_  SignalName  :  8|16@1+  (0.1,0)  [0|655.35]  \"km/h\"  ECU1,ECU2";
  
  auto result = SignalParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "SignalName");
  EXPECT_EQ(result->start_bit, 8);
}

TEST_F(SignalParserTest, HandlesNoReceivingECUs) {
  const std::string kInput = "SG_ Speed : 8|16@1+ (0.1,0) [0|655.35] \"km/h\" Vector__XXX";
  
  auto result = SignalParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "Speed");
  EXPECT_THAT(result->receivers, ElementsAre("Vector__XXX"));
}

TEST_F(SignalParserTest, HandlesEmptyUnit) {
  const std::string kInput = "SG_ Status : 0|8@1+ (1,0) [0|255] \"\" ECU1";
  
  auto result = SignalParser::Parse(kInput);
  ASSERT_TRUE(result.has_value());
  
  EXPECT_EQ(result->name, "Status");
  EXPECT_EQ(result->unit, "");
}

TEST_F(SignalParserTest, RejectsInvalidFormat) {
  const std::vector<std::string> kInvalidInputs = {
    // Missing SG_ prefix
    "SignalName : 8|16@1+ (0.1,0) [0|655.35] \"km/h\" ECU1",
    // Missing colon
    "SG_ SignalName 8|16@1+ (0.1,0) [0|655.35] \"km/h\" ECU1",
    // Invalid bit position format
    "SG_ SignalName : A|16@1+ (0.1,0) [0|655.35] \"km/h\" ECU1",
    // Invalid bit size
    "SG_ SignalName : 8|B@1+ (0.1,0) [0|655.35] \"km/h\" ECU1",
    // Invalid byte order
    "SG_ SignalName : 8|16@2+ (0.1,0) [0|655.35] \"km/h\" ECU1",
    // Invalid sign
    "SG_ SignalName : 8|16@1* (0.1,0) [0|655.35] \"km/h\" ECU1",
    // Missing factor/offset parentheses
    "SG_ SignalName : 8|16@1+ 0.1,0 [0|655.35] \"km/h\" ECU1",
    // Missing min/max brackets
    "SG_ SignalName : 8|16@1+ (0.1,0) 0|655.35 \"km/h\" ECU1",
    // Missing unit quotes
    "SG_ SignalName : 8|16@1+ (0.1,0) [0|655.35] km/h ECU1",
    // Empty input
    ""
  };
  
  for (const auto& input : kInvalidInputs) {
    EXPECT_FALSE(SignalParser::Parse(input).has_value()) 
      << "Input should be rejected: " << input;
  }
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 