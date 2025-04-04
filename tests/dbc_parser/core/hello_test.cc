#include "gtest/gtest.h"
#include "src/dbc_parser/core/hello.h"

namespace dbc_parser {
namespace core {
namespace {

TEST(HelloTest, ReturnsExpectedGreeting) {
  const std::string expected = "Hello from DBC Parser!";
  EXPECT_EQ(GetGreeting(), expected);
}

}  // namespace
}  // namespace core
}  // namespace dbc_parser 