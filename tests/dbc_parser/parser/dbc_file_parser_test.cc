#include "src/dbc_parser/parser/dbc_file_parser.h"

#include <string>
#include <string_view>

#include "gtest/gtest.h"

namespace dbc_parser {
namespace parser {
namespace {

// Test fixture for DBC file parser
class DbcFileParserTest : public ::testing::Test {
 protected:
  void SetUp() override {
    parser_ = std::make_unique<DbcFileParser>();
  }

  std::unique_ptr<DbcFileParser> parser_;
};

// Test basic version parsing
TEST_F(DbcFileParserTest, ParsesVersion) {
  const std::string kInput = "VERSION \"1.0\"\n";

  auto result = parser_->Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("1.0", result->version);
}

// Test handling invalid version format
TEST_F(DbcFileParserTest, HandlesInvalidVersionFormat) {
  const std::string kInput = "VERSION 1.0\n";  // Missing quotes

  auto result = parser_->Parse(kInput);
  EXPECT_FALSE(result.has_value());
}

// Test parsing with multiple sections
TEST_F(DbcFileParserTest, HandlesCombinedSections) {
  const std::string kInput = R"(
VERSION "2.0"

NS_ : 
    NS_DESC_
    CM_
)";

  auto result = parser_->Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("2.0", result->version);
}

// Test parsing multiple sections
TEST_F(DbcFileParserTest, ParsesMultipleSections) {
  constexpr std::string_view kInput = R"(
VERSION "2.0"

NS_ : 
    NS_DESC_
    CM_
    BA_DEF_
    BA_
    VAL_
    CAT_DEF_
    CAT_
    FILTER
    BA_DEF_DEF_
    EV_DATA_
    ENVVAR_DATA_
    SGTYPE_
    SGTYPE_VAL_
    BA_DEF_SGTYPE_
    BA_SGTYPE_
    SIG_TYPE_REF_
    VAL_TABLE_
    SIG_GROUP_
    SIG_VALTYPE_
    SIGTYPE_VALTYPE_
    BO_TX_BU_
    BA_DEF_REL_
    BA_REL_
    BA_DEF_DEF_REL_
    BU_SG_REL_
    BU_EV_REL_
    BU_BO_REL_
    SG_MUL_VAL_

BS_:

BU_: Node1 Node2
)";

  auto result = parser_->Parse(kInput);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("2.0", result->version);
  // Add more assertions for other sections
}

// Add more test cases for different DBC file elements

}  // namespace
}  // namespace parser
}  // namespace dbc_parser 