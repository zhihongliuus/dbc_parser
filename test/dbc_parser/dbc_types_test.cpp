#include "dbc_parser/types.h"
#include "dbc_parser/parser.h"
#include "dbc_parser/decoder.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

namespace dbc_parser {
namespace testing {

class DbcTypesTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Create a database with various DBC data types
    db_ = std::make_unique<Database>();
  }

  std::unique_ptr<Database> db_;
};

// Test new symbols
TEST_F(DbcTypesTest, NewSymbols) {
  std::vector<std::string> symbols = {"Symbol1", "Symbol2", "Symbol3"};
  db_->set_new_symbols(symbols);

  EXPECT_EQ(db_->new_symbols().size(), 3);
  EXPECT_EQ(db_->new_symbols()[0], "Symbol1");
  EXPECT_EQ(db_->new_symbols()[1], "Symbol2");
  EXPECT_EQ(db_->new_symbols()[2], "Symbol3");
}

// Test value tables
TEST_F(DbcTypesTest, ValueTables) {
  auto value_table = std::make_unique<ValueTable>("TestTable");
  value_table->add_value(1, "Value1");
  value_table->add_value(2, "Value2");
  
  ValueTable* table_ptr = value_table.get();
  db_->add_value_table(std::move(value_table));
  
  EXPECT_EQ(db_->value_tables().size(), 1);
  EXPECT_EQ(db_->get_value_table("TestTable"), table_ptr);
  EXPECT_EQ(table_ptr->values().size(), 2);
  EXPECT_EQ(table_ptr->values().at(1), "Value1");
  EXPECT_EQ(table_ptr->values().at(2), "Value2");
}

// Test environment variables
TEST_F(DbcTypesTest, EnvironmentVariables) {
  std::vector<std::string> access_nodes = {"Node1", "Node2"};
  auto env_var = std::make_unique<EnvironmentVariable>(
      "TestEnvVar", EnvVarType::kInteger, 
      0.0, 100.0, "km/h", 50.0, 123, 
      EnvVarAccessType::kReadWrite, access_nodes);
  
  env_var->add_data_value(10, "Low");
  env_var->add_data_value(50, "Medium");
  env_var->add_data_value(90, "High");
  
  EnvironmentVariable* env_ptr = env_var.get();
  db_->add_environment_variable(std::move(env_var));
  
  EXPECT_EQ(db_->environment_variables().size(), 1);
  EXPECT_EQ(db_->get_environment_variable("TestEnvVar"), env_ptr);
  EXPECT_EQ(env_ptr->type(), EnvVarType::kInteger);
  EXPECT_EQ(env_ptr->min_value(), 0.0);
  EXPECT_EQ(env_ptr->max_value(), 100.0);
  EXPECT_EQ(env_ptr->unit(), "km/h");
  EXPECT_EQ(env_ptr->initial_value(), 50.0);
  EXPECT_EQ(env_ptr->ev_id(), 123);
  EXPECT_EQ(env_ptr->access_type(), EnvVarAccessType::kReadWrite);
  EXPECT_EQ(env_ptr->access_nodes().size(), 2);
  EXPECT_EQ(env_ptr->data_values().size(), 3);
  EXPECT_EQ(env_ptr->data_values().at(10), "Low");
  EXPECT_EQ(env_ptr->data_values().at(50), "Medium");
  EXPECT_EQ(env_ptr->data_values().at(90), "High");
}

// Test signal types
TEST_F(DbcTypesTest, SignalTypes) {
  auto signal_type = std::make_unique<SignalType>(
      "TestSignalType", 0.0, 100.0, "km/h", 
      0.1, 0.0, 16, true);
  
  signal_type->set_value_table("TestValueTable");
  
  SignalType* type_ptr = signal_type.get();
  db_->add_signal_type(std::move(signal_type));
  
  EXPECT_EQ(db_->signal_types().size(), 1);
  EXPECT_EQ(db_->get_signal_type("TestSignalType"), type_ptr);
  EXPECT_EQ(type_ptr->min_value(), 0.0);
  EXPECT_EQ(type_ptr->max_value(), 100.0);
  EXPECT_EQ(type_ptr->unit(), "km/h");
  EXPECT_EQ(type_ptr->factor(), 0.1);
  EXPECT_EQ(type_ptr->offset(), 0.0);
  EXPECT_EQ(type_ptr->length(), 16);
  EXPECT_TRUE(type_ptr->is_signed());
  EXPECT_EQ(type_ptr->value_table(), "TestValueTable");
}

// Test attributes
TEST_F(DbcTypesTest, Attributes) {
  // Create attribute definitions
  auto attr_def = std::make_unique<AttributeDefinition>(
      "TestAttr", AttributeType::kEnum);
  
  attr_def->set_min(0);
  attr_def->set_max(10);
  attr_def->add_enum_value(0, "Zero");
  attr_def->add_enum_value(1, "One");
  attr_def->add_enum_value(2, "Two");
  
  AttributeDefinition* attr_ptr = attr_def.get();
  db_->add_attribute_definition(std::move(attr_def));
  
  EXPECT_EQ(db_->attribute_definitions().size(), 1);
  EXPECT_EQ(db_->get_attribute_definition("TestAttr"), attr_ptr);
  EXPECT_EQ(attr_ptr->type(), AttributeType::kEnum);
  EXPECT_EQ(std::get<int>(attr_ptr->min()), 0);
  EXPECT_EQ(std::get<int>(attr_ptr->max()), 10);
  EXPECT_EQ(attr_ptr->enum_values().size(), 3);
  EXPECT_EQ(attr_ptr->enum_values().at(0), "Zero");
  EXPECT_EQ(attr_ptr->enum_values().at(1), "One");
  EXPECT_EQ(attr_ptr->enum_values().at(2), "Two");
  
  // Test attribute defaults
  db_->set_attribute_default("DefaultAttr", 42);
  EXPECT_EQ(db_->attribute_defaults().size(), 1);
  EXPECT_EQ(std::get<int>(db_->attribute_defaults().at("DefaultAttr")), 42);
  
  // Test global attributes
  db_->set_global_attribute("GlobalAttr", "global_value");
  EXPECT_EQ(db_->global_attributes().size(), 1);
  EXPECT_EQ(std::get<std::string>(db_->global_attributes().at("GlobalAttr")), "global_value");
  
  // Test node attributes
  db_->set_node_attribute("Node1", "NodeAttr", 3.14);
  EXPECT_EQ(db_->node_attributes().size(), 1);
  EXPECT_EQ(db_->node_attributes().at("Node1").size(), 1);
  EXPECT_DOUBLE_EQ(std::get<double>(db_->node_attributes().at("Node1").at("NodeAttr")), 3.14);
  
  // Test message attributes
  db_->set_message_attribute(123, "MsgAttr", 987);
  EXPECT_EQ(db_->message_attributes().size(), 1);
  EXPECT_EQ(db_->message_attributes().at(123).size(), 1);
  EXPECT_EQ(std::get<int>(db_->message_attributes().at(123).at("MsgAttr")), 987);
  
  // Test signal attributes
  db_->set_signal_attribute(123, "Signal1", "SigAttr", "signal_value");
  EXPECT_EQ(db_->signal_attributes().size(), 1);
  EXPECT_EQ(db_->signal_attributes().at(123).size(), 1);
  EXPECT_EQ(db_->signal_attributes().at(123).at("Signal1").size(), 1);
  EXPECT_EQ(std::get<std::string>(db_->signal_attributes().at(123).at("Signal1").at("SigAttr")), "signal_value");
}

// Test signal type references
TEST_F(DbcTypesTest, SignalTypeReferences) {
  db_->set_signal_type_ref(123, "Signal1", "TypeRef1");
  EXPECT_EQ(db_->signal_type_refs().size(), 1);
  EXPECT_EQ(db_->signal_type_refs().at(123).size(), 1);
  EXPECT_EQ(db_->signal_type_refs().at(123).at("Signal1"), "TypeRef1");
}

// Test signal multiplexer values
TEST_F(DbcTypesTest, SignalMultiplexerValues) {
  // Create a message with multiplexed signals
  auto msg = std::make_unique<Message>(123, "MultiplexedMsg", 8, "ECU1");
  
  // Add the multiplexor signal
  auto mux_signal = std::make_unique<Signal>("MuxSelector", 0, 4, true, false, 1.0, 0.0, 0.0, 15.0, "");
  mux_signal->set_mux_type(MultiplexerType::kMultiplexor);
  EXPECT_EQ(MultiplexerType::kMultiplexor, mux_signal->mux_type());
  
  msg->add_signal(std::move(mux_signal));
  
  // Add multiplexed signals for different selector values
  auto signal1 = std::make_unique<Signal>("Signal1", 8, 8, true, false, 1.0, 0.0, 0.0, 255.0, "");
  signal1->set_mux_type(MultiplexerType::kMultiplexed);
  signal1->set_mux_value(0);
  EXPECT_EQ(MultiplexerType::kMultiplexed, signal1->mux_type());
  EXPECT_EQ(0, signal1->mux_value());
  
  auto signal2 = std::make_unique<Signal>("Signal2", 8, 8, true, false, 1.0, 0.0, 0.0, 255.0, "");
  signal2->set_mux_type(MultiplexerType::kMultiplexed);
  signal2->set_mux_value(1);
  EXPECT_EQ(MultiplexerType::kMultiplexed, signal2->mux_type());
  EXPECT_EQ(1, signal2->mux_value());
  
  msg->add_signal(std::move(signal1));
  msg->add_signal(std::move(signal2));
  
  db_->add_message(std::move(msg));
  
  // Retrieve the message and check the multiplexed signals
  Message* retrieved_msg = db_->get_message(123);
  ASSERT_NE(nullptr, retrieved_msg);
  
  Signal* mux = retrieved_msg->get_signal("MuxSelector");
  ASSERT_NE(nullptr, mux);
  EXPECT_EQ(MultiplexerType::kMultiplexor, mux->mux_type());
  
  Signal* sig1 = retrieved_msg->get_signal("Signal1");
  ASSERT_NE(nullptr, sig1);
  EXPECT_EQ(MultiplexerType::kMultiplexed, sig1->mux_type());
  EXPECT_EQ(0, sig1->mux_value());
  
  Signal* sig2 = retrieved_msg->get_signal("Signal2");
  ASSERT_NE(nullptr, sig2);
  EXPECT_EQ(MultiplexerType::kMultiplexed, sig2->mux_type());
  EXPECT_EQ(1, sig2->mux_value());
}

// Test signal extended value types
TEST_F(DbcTypesTest, SignalExtendedValueTypes) {
  // Create a message with signals using extended value types
  auto msg = std::make_unique<Message>(123, "ExtendedValueTypeMsg", 8, "ECU1");
  
  // Add signals with different extended value types
  auto signal1 = std::make_unique<Signal>("FloatSignal", 0, 32, true, true, 1.0, 0.0, 0.0, 0.0, "");
  signal1->set_extended_value_type(SignalExtendedValueType::kFloat);
  EXPECT_EQ(SignalExtendedValueType::kFloat, signal1->extended_value_type());
  
  auto signal2 = std::make_unique<Signal>("DoubleSignal", 32, 64, true, true, 1.0, 0.0, 0.0, 0.0, "");
  signal2->set_extended_value_type(SignalExtendedValueType::kDouble);
  EXPECT_EQ(SignalExtendedValueType::kDouble, signal2->extended_value_type());
  
  msg->add_signal(std::move(signal1));
  msg->add_signal(std::move(signal2));
  
  db_->add_message(std::move(msg));
  
  // Retrieve the message and check the extended value types
  Message* retrieved_msg = db_->get_message(123);
  ASSERT_NE(nullptr, retrieved_msg);
  
  Signal* float_sig = retrieved_msg->get_signal("FloatSignal");
  ASSERT_NE(nullptr, float_sig);
  EXPECT_EQ(SignalExtendedValueType::kFloat, float_sig->extended_value_type());
  
  Signal* double_sig = retrieved_msg->get_signal("DoubleSignal");
  ASSERT_NE(nullptr, double_sig);
  EXPECT_EQ(SignalExtendedValueType::kDouble, double_sig->extended_value_type());
}

// Test signal groups
TEST_F(DbcTypesTest, SignalGroups) {
  // Create a message with signal groups
  auto msg = std::make_unique<Message>(123, "GroupedSignalsMsg", 8, "ECU1");
  
  // Add some signals
  auto signal1 = std::make_unique<Signal>("Signal1", 0, 8, true, false, 1.0, 0.0, 0.0, 255.0, "");
  auto signal2 = std::make_unique<Signal>("Signal2", 8, 8, true, false, 1.0, 0.0, 0.0, 255.0, "");
  auto signal3 = std::make_unique<Signal>("Signal3", 16, 8, true, false, 1.0, 0.0, 0.0, 255.0, "");
  
  msg->add_signal(std::move(signal1));
  msg->add_signal(std::move(signal2));
  msg->add_signal(std::move(signal3));
  
  // Create a signal group
  auto group = std::make_unique<SignalGroup>(123, "BasicGroup", 1);
  group->add_signal("Signal1");
  group->add_signal("Signal2");
  
  EXPECT_EQ(123, group->message_id());
  EXPECT_EQ("BasicGroup", group->name());
  EXPECT_EQ(1, group->id());
  EXPECT_EQ(2, group->signals().size());
  EXPECT_EQ("Signal1", group->signals()[0]);
  EXPECT_EQ("Signal2", group->signals()[1]);
  
  msg->add_signal_group(std::move(group));
  EXPECT_EQ(1, msg->signal_groups().size());
  
  db_->add_message(std::move(msg));
  
  // Retrieve the message and check the signal group
  Message* retrieved_msg = db_->get_message(123);
  ASSERT_NE(nullptr, retrieved_msg);
  EXPECT_EQ(1, retrieved_msg->signal_groups().size());
  
  const SignalGroup* retrieved_group = retrieved_msg->signal_groups()[0].get();
  ASSERT_NE(nullptr, retrieved_group);
  EXPECT_EQ("BasicGroup", retrieved_group->name());
  EXPECT_EQ(2, retrieved_group->signals().size());
}

} // namespace testing
} // namespace dbc_parser

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
} 