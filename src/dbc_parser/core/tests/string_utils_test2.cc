#include "gtest/gtest.h"
#include "../string_utils.h"
TEST(StringUtilsTest2, TrimWorks) {
  EXPECT_EQ(dbc_parser::core::StringUtils::Trim("  hello  "), "hello");
}
