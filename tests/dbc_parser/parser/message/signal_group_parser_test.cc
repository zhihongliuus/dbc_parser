#include "src/dbc_parser/parser/message/signal_group_parser.h"

#include <string>
#include <optional>
#include <vector>

#include "gtest/gtest.h"

namespace dbc_parser {
namespace parser {
namespace {

TEST(SignalGroupParserTest, ParsesBasicSignalGroup) {
  const std::string kInput = "SIG_GROUP_ 500 EngineData 1 : Rpm,Temp,Pressure;";
  auto result = SignalGroupParser::Parse(kInput);
  
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->message_id, 500);
  EXPECT_EQ(result->group_name, "EngineData");
  EXPECT_EQ(result->repetitions, 1);
  
  ASSERT_EQ(result->signals.size(), 3);
  EXPECT_EQ(result->signals[0], "Rpm");
  EXPECT_EQ(result->signals[1], "Temp");
  EXPECT_EQ(result->signals[2], "Pressure");
}

TEST(SignalGroupParserTest, ParsesNegativeMessageId) {
  const std::string kInput = "SIG_GROUP_ -123 TestGroup 2 : Signal1,Signal2;";
  auto result = SignalGroupParser::Parse(kInput);
  
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->message_id, -123);
  EXPECT_EQ(result->group_name, "TestGroup");
  EXPECT_EQ(result->repetitions, 2);
  
  ASSERT_EQ(result->signals.size(), 2);
  EXPECT_EQ(result->signals[0], "Signal1");
  EXPECT_EQ(result->signals[1], "Signal2");
}

TEST(SignalGroupParserTest, ParsesSingleSignal) {
  const std::string kInput = "SIG_GROUP_ 42 SingleSignalGroup 3 : JustOneSignal;";
  auto result = SignalGroupParser::Parse(kInput);
  
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->message_id, 42);
  EXPECT_EQ(result->group_name, "SingleSignalGroup");
  EXPECT_EQ(result->repetitions, 3);
  
  ASSERT_EQ(result->signals.size(), 1);
  EXPECT_EQ(result->signals[0], "JustOneSignal");
}

TEST(SignalGroupParserTest, HandlesWhitespace) {
  const std::string kInput = "SIG_GROUP_  1234   SpacedGroup   5   :  Sig1 , Sig2 , Sig3  ;";
  auto result = SignalGroupParser::Parse(kInput);
  
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->message_id, 1234);
  EXPECT_EQ(result->group_name, "SpacedGroup");
  EXPECT_EQ(result->repetitions, 5);
  
  ASSERT_EQ(result->signals.size(), 3);
  EXPECT_EQ(result->signals[0], "Sig1");
  EXPECT_EQ(result->signals[1], "Sig2");
  EXPECT_EQ(result->signals[2], "Sig3");
}

TEST(SignalGroupParserTest, HandlesUnderscoresInNames) {
  const std::string kInput = "SIG_GROUP_ 555 Group_With_Underscores 1 : Signal_1,Another_Signal;";
  auto result = SignalGroupParser::Parse(kInput);
  
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->message_id, 555);
  EXPECT_EQ(result->group_name, "Group_With_Underscores");
  EXPECT_EQ(result->repetitions, 1);
  
  ASSERT_EQ(result->signals.size(), 2);
  EXPECT_EQ(result->signals[0], "Signal_1");
  EXPECT_EQ(result->signals[1], "Another_Signal");
}

TEST(SignalGroupParserTest, RejectsInvalidFormat) {
  // Missing keyword
  EXPECT_FALSE(SignalGroupParser::Parse("500 EngineData 1 : Rpm,Temp,Pressure;").has_value());
  
  // Missing message ID
  EXPECT_FALSE(SignalGroupParser::Parse("SIG_GROUP_ EngineData 1 : Rpm,Temp,Pressure;").has_value());
  
  // Invalid message ID
  EXPECT_FALSE(SignalGroupParser::Parse("SIG_GROUP_ XYZ EngineData 1 : Rpm,Temp,Pressure;").has_value());
  
  // Missing group name
  EXPECT_FALSE(SignalGroupParser::Parse("SIG_GROUP_ 500 1 : Rpm,Temp,Pressure;").has_value());
  
  // Missing repetitions
  EXPECT_FALSE(SignalGroupParser::Parse("SIG_GROUP_ 500 EngineData : Rpm,Temp,Pressure;").has_value());
  
  // Invalid repetitions
  EXPECT_FALSE(SignalGroupParser::Parse("SIG_GROUP_ 500 EngineData ABC : Rpm,Temp,Pressure;").has_value());
  
  // Missing colon
  EXPECT_FALSE(SignalGroupParser::Parse("SIG_GROUP_ 500 EngineData 1 Rpm,Temp,Pressure;").has_value());
  
  // Missing semicolon
  EXPECT_FALSE(SignalGroupParser::Parse("SIG_GROUP_ 500 EngineData 1 : Rpm,Temp,Pressure").has_value());
  
  // Missing signal list
  EXPECT_FALSE(SignalGroupParser::Parse("SIG_GROUP_ 500 EngineData 1 : ;").has_value());
}

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 