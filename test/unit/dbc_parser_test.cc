#include <gtest/gtest.h>

#include "src/dbc_parser.h"

namespace dbc_parser {
namespace {

TEST(DbcParserTest, Initialization) {
  DbcParser parser;
  EXPECT_EQ("", parser.GetLastError());
}

// More tests will be added as the implementation progresses

}  // namespace
}  // namespace dbc_parser 