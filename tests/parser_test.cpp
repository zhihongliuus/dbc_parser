#include "dbc_parser/parser.h"
#include "dbc_parser/types.h"

#include <gtest/gtest.h>
#include <string>
#include <memory>

namespace {

// Simple test fixture for DBC parser tests
class DbcParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code here
    }

    void TearDown() override {
        // Teardown code here
    }

    dbc_parser::DbcParser parser_;
};

// Test parsing a simple DBC string
TEST_F(DbcParserTest, ParseSimpleDbcString) {
    const std::string dbc_content = R"(
VERSION "1.0"

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

BS_: 500000,8,1

BU_: Node1 Node2

BO_ 123 TestMessage: 8 Node1
 SG_ TestSignal : 0|8@1+ (1,0) [0|255] "" Node2

CM_ SG_ 123 TestSignal "Test signal comment";
)";

    // Parse the DBC string
    auto database = parser_.parse_string(dbc_content);
    
    // Verify the parsed data
    ASSERT_NE(database, nullptr);
    
    // Check version
    ASSERT_NE(database->version(), nullptr);
    EXPECT_EQ(database->version()->version, "1.0");
    
    // Check bit timing
    ASSERT_NE(database->bit_timing(), nullptr);
    EXPECT_EQ(database->bit_timing()->baudrate, 500000);
    
    // Check nodes
    ASSERT_EQ(database->nodes().size(), 2);
    EXPECT_EQ(database->nodes()[0]->name(), "Node1");
    EXPECT_EQ(database->nodes()[1]->name(), "Node2");
    
    // Check messages
    ASSERT_EQ(database->messages().size(), 1);
    EXPECT_EQ(database->messages()[0]->id(), 123);
    EXPECT_EQ(database->messages()[0]->name(), "TestMessage");
    EXPECT_EQ(database->messages()[0]->length(), 8);
    EXPECT_EQ(database->messages()[0]->sender(), "Node1");
    
    // Check signals
    auto message = database->messages()[0].get();
    ASSERT_EQ(message->signals().size(), 1);
    
    auto signal = message->get_signal("TestSignal");
    ASSERT_NE(signal, nullptr);
    EXPECT_EQ(signal->name(), "TestSignal");
    EXPECT_EQ(signal->start_bit(), 0);
    EXPECT_EQ(signal->length(), 8);
    EXPECT_TRUE(signal->is_little_endian());
    EXPECT_FALSE(signal->is_signed());
    EXPECT_EQ(signal->factor(), 1.0);
    EXPECT_EQ(signal->offset(), 0.0);
    EXPECT_EQ(signal->min_value(), 0.0);
    EXPECT_EQ(signal->max_value(), 255.0);
    EXPECT_EQ(signal->unit(), "");
    EXPECT_EQ(signal->comment(), "Test signal comment");
    
    // Check receivers
    ASSERT_EQ(signal->receivers().size(), 1);
    EXPECT_EQ(signal->receivers()[0], "Node2");
}

} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 