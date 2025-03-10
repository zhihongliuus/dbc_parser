#include "../include/dbc_parser/types.h"
#include "../include/dbc_parser/signal_decoder.h"
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <cstdint>

namespace dbc_parser {

//------------------------------------------------------------------------------
// Signal implementation
//------------------------------------------------------------------------------

Signal::Signal(const std::string& name, uint32_t start_bit, uint32_t length,
               bool is_little_endian, bool is_signed, double factor, double offset,
               double min_value, double max_value, const std::string& unit)
    : name_(name),
      start_bit_(start_bit),
      length_(length),
      is_little_endian_(is_little_endian),
      is_signed_(is_signed),
      factor_(factor),
      offset_(offset),
      min_value_(min_value),
      max_value_(max_value),
      unit_(unit),
      mux_type_(MultiplexerType::kNone),
      mux_value_(0) {
}

const std::string& Signal::name() const {
  return name_;
}

uint32_t Signal::start_bit() const {
  return start_bit_;
}

uint32_t Signal::length() const {
  return length_;
}

bool Signal::is_little_endian() const {
  return is_little_endian_;
}

bool Signal::is_signed() const {
  return is_signed_;
}

double Signal::factor() const {
  return factor_;
}

double Signal::offset() const {
  return offset_;
}

double Signal::min_value() const {
  return min_value_;
}

double Signal::max_value() const {
  return max_value_;
}

const std::string& Signal::unit() const {
  return unit_;
}

const std::string& Signal::comment() const {
  return comment_;
}

MultiplexerType Signal::mux_type() const {
  return mux_type_;
}

uint32_t Signal::mux_value() const {
  return mux_value_;
}

const std::vector<std::string>& Signal::receivers() const {
  return receivers_;
}

const std::map<uint64_t, std::string>& Signal::value_descriptions() const {
  return value_descriptions_;
}

void Signal::set_comment(const std::string& comment) {
  comment_ = comment;
}

void Signal::set_mux_type(MultiplexerType type) {
  mux_type_ = type;
}

void Signal::set_mux_value(uint32_t value) {
  mux_value_ = value;
}

void Signal::set_factor(double factor) {
  factor_ = factor;
}

void Signal::set_offset(double offset) {
  offset_ = offset;
}

void Signal::add_receiver(const std::string& receiver) {
  receivers_.push_back(receiver);
}

void Signal::add_value_description(uint64_t value, const std::string& description) {
  value_descriptions_[value] = description;
}

void Signal::set_attribute(const std::string& name, const Attribute& attribute) {
  attributes_[name] = attribute;
}

double Signal::decode(const std::vector<uint8_t>& data) const {
  return SignalDecoder::decode(
    data,
    start_bit_,
    length_,
    is_little_endian_,
    is_signed_,
    factor_,
    offset_
  );
}

void Signal::encode(double value, std::vector<uint8_t>& data) const {
  SignalDecoder::encode(
    value,
    data,
    start_bit_,
    length_,
    is_little_endian_,
    is_signed_,
    factor_,
    offset_
  );
}

//------------------------------------------------------------------------------
// Message implementation
//------------------------------------------------------------------------------

Message::Message(MessageId id, const std::string& name, uint32_t length, const std::string& sender)
    : id_(id),
      name_(name),
      length_(length),
      sender_(sender) {
}

Message::Message(const Message& other)
    : id_(other.id_),
      name_(other.name_),
      length_(other.length_),
      sender_(other.sender_),
      comment_(other.comment_),
      transmitters_(other.transmitters_),
      attributes_(other.attributes_) {
  // Deep copy the signals
  for (const auto& signal_pair : other.signals_) {
    // Create a new Signal with the same properties
    std::unique_ptr<Signal> new_signal = std::make_unique<Signal>(
        signal_pair.second->name(),
        signal_pair.second->start_bit(),
        signal_pair.second->length(),
        signal_pair.second->is_little_endian(),
        signal_pair.second->is_signed(),
        signal_pair.second->factor(),
        signal_pair.second->offset(),
        signal_pair.second->min_value(),
        signal_pair.second->max_value(),
        signal_pair.second->unit());
        
    // Copy other properties
    new_signal->set_comment(signal_pair.second->comment());
    new_signal->set_mux_type(signal_pair.second->mux_type());
    new_signal->set_mux_value(signal_pair.second->mux_value());
    
    // Add receivers
    for (const auto& receiver : signal_pair.second->receivers()) {
      new_signal->add_receiver(receiver);
    }
    
    // Add value descriptions
    for (const auto& desc_pair : signal_pair.second->value_descriptions()) {
      new_signal->add_value_description(desc_pair.first, desc_pair.second);
    }
    
    // Add to signals map
    signals_[signal_pair.first] = std::move(new_signal);
  }
  
  // Deep copy the signal groups
  for (const auto& group_pair : other.signal_groups_) {
    std::unique_ptr<SignalGroup> new_group = std::make_unique<SignalGroup>(
        group_pair.second->name(),
        group_pair.second->id());
    
    signal_groups_[group_pair.first] = std::move(new_group);
  }
}

MessageId Message::id() const {
  return id_;
}

const std::string& Message::name() const {
  return name_;
}

uint32_t Message::length() const {
  return length_;
}

const std::string& Message::sender() const {
  return sender_;
}

const std::string& Message::comment() const {
  return comment_;
}

const std::map<std::string, std::unique_ptr<Signal>>& Message::signals() const {
  return signals_;
}

const std::vector<std::string>& Message::transmitters() const {
  return transmitters_;
}

void Message::set_comment(const std::string& comment) {
  comment_ = comment;
}

Signal* Message::add_signal(std::unique_ptr<Signal> signal) {
  const std::string& name = signal->name();
  Signal* signal_ptr = signal.get();
  signals_[name] = std::move(signal);
  return signal_ptr;
}

Signal* Message::get_signal(const std::string& name) const {
  auto it = signals_.find(name);
  if (it != signals_.end()) {
    return it->second.get();
  }
  return nullptr;
}

bool Message::remove_signal(const std::string& name) {
  return signals_.erase(name) > 0;
}

void Message::set_attribute(const std::string& name, const Attribute& attribute) {
  attributes_[name] = attribute;
}

void Message::add_transmitter(const std::string& transmitter) {
  transmitters_.push_back(transmitter);
}

void Message::add_signal_group(std::unique_ptr<SignalGroup> group) {
  const std::string& name = group->name();
  signal_groups_[name] = std::move(group);
}

std::map<std::string, double> Message::decode(const std::vector<uint8_t>& data) const {
  std::map<std::string, double> result;
  
  for (const auto& signal_pair : signals_) {
    // Skip multiplexed signals that don't match the multiplexer value
    if (signal_pair.second->mux_type() == MultiplexerType::kMultiplexed) {
      // Find the multiplexer signal
      for (const auto& mux_signal_pair : signals_) {
        if (mux_signal_pair.second->mux_type() == MultiplexerType::kMultiplexor) {
          double mux_value = mux_signal_pair.second->decode(data);
          
          // If the multiplexer value doesn't match, skip this signal
          if (static_cast<uint32_t>(mux_value) != signal_pair.second->mux_value()) {
            continue;
          }
          break;
        }
      }
    }
    
    // Decode the signal value
    result[signal_pair.first] = signal_pair.second->decode(data);
  }
  
  return result;
}

//------------------------------------------------------------------------------
// Node implementation
//------------------------------------------------------------------------------

Node::Node(const std::string& name)
    : name_(name) {
}

const std::string& Node::name() const {
  return name_;
}

const std::string& Node::comment() const {
  return comment_;
}

void Node::set_comment(const std::string& comment) {
  comment_ = comment;
}

void Node::set_attribute(const std::string& name, const Attribute& attribute) {
  attributes_[name] = attribute;
}

//------------------------------------------------------------------------------
// ValueTable implementation
//------------------------------------------------------------------------------

ValueTable::ValueTable(const std::string& name)
    : name_(name) {
}

const std::string& ValueTable::name() const {
  return name_;
}

const std::map<uint64_t, std::string>& ValueTable::entries() const {
  return entries_;
}

void ValueTable::add_entry(uint64_t value, const std::string& description) {
  entries_[value] = description;
}

//------------------------------------------------------------------------------
// Attribute implementation
//------------------------------------------------------------------------------

Attribute::Attribute(const std::string& name, const std::string& value)
    : name_(name), value_(value) {
}

const std::string& Attribute::name() const {
  return name_;
}

const std::string& Attribute::value() const {
  return value_;
}

void Attribute::set_value(const std::string& value) {
  value_ = value;
}

//------------------------------------------------------------------------------
// SignalGroup implementation
//------------------------------------------------------------------------------

SignalGroup::SignalGroup(const std::string& name, uint32_t id)
    : name_(name), id_(id) {
}

const std::string& SignalGroup::name() const {
  return name_;
}

uint32_t SignalGroup::id() const {
  return id_;
}

void SignalGroup::add_signal(const std::string& signal_name) {
  signals_.push_back(signal_name);
}

//------------------------------------------------------------------------------
// EnvironmentVariable implementation
//------------------------------------------------------------------------------

EnvironmentVariable::EnvironmentVariable(const std::string& name, uint32_t type, double min, double max)
    : name_(name), type_(type), min_(min), max_(max) {
}

const std::string& EnvironmentVariable::name() const {
  return name_;
}

uint32_t EnvironmentVariable::type() const {
  return type_;
}

double EnvironmentVariable::min() const {
  return min_;
}

double EnvironmentVariable::max() const {
  return max_;
}

//------------------------------------------------------------------------------
// SignalType implementation
//------------------------------------------------------------------------------

SignalType::SignalType(const std::string& name)
    : name_(name) {
}

const std::string& SignalType::name() const {
  return name_;
}

//------------------------------------------------------------------------------
// AttributeDefinition implementation
//------------------------------------------------------------------------------

AttributeDefinition::AttributeDefinition(const std::string& name, Type type)
    : name_(name), type_(type) {
}

const std::string& AttributeDefinition::name() const {
  return name_;
}

AttributeDefinition::Type AttributeDefinition::type() const {
  return type_;
}

//------------------------------------------------------------------------------
// Database implementation
//------------------------------------------------------------------------------

Database::Database() {
}

Database::Database(const Database& other) {
  // Copy version and bit_timing if they exist
  if (other.version_) {
    version_ = other.version_;
  }
  if (other.bit_timing_) {
    bit_timing_ = other.bit_timing_;
  }
  
  // Deep copy nodes
  for (const auto& node_pair : other.nodes_) {
    auto new_node = std::make_unique<Node>(node_pair.second->name());
    new_node->set_comment(node_pair.second->comment());
    nodes_[node_pair.first] = std::move(new_node);
  }
  
  // Deep copy value tables
  for (const auto& table_pair : other.value_tables_) {
    auto new_table = std::make_unique<ValueTable>(table_pair.second->name());
    for (const auto& entry : table_pair.second->entries()) {
      new_table->add_entry(entry.first, entry.second);
    }
    value_tables_[table_pair.first] = std::move(new_table);
  }
  
  // Deep copy messages
  for (const auto& message_pair : other.messages_) {
    // Use the Message copy constructor which already handles deep copying
    auto new_message = std::make_unique<Message>(*message_pair.second);
    messages_[message_pair.first] = std::move(new_message);
  }
  
  // Deep copy environment variables
  for (const auto& env_var_pair : other.env_vars_) {
    auto new_env_var = std::make_unique<EnvironmentVariable>(
        env_var_pair.second->name(),
        env_var_pair.second->type(),
        env_var_pair.second->min(),
        env_var_pair.second->max());
    env_vars_[env_var_pair.first] = std::move(new_env_var);
  }
  
  // Deep copy signal types
  for (const auto& signal_type_pair : other.signal_types_) {
    auto new_signal_type = std::make_unique<SignalType>(signal_type_pair.second->name());
    signal_types_[signal_type_pair.first] = std::move(new_signal_type);
  }
  
  // Deep copy attribute definitions
  for (const auto& attr_def_pair : other.attribute_defs_) {
    auto new_attr_def = std::make_unique<AttributeDefinition>(
        attr_def_pair.second->name(),
        attr_def_pair.second->type());
    attribute_defs_[attr_def_pair.first] = std::move(new_attr_def);
  }
}

const std::optional<Database::Version>& Database::version() const {
  return version_;
}

const std::optional<Database::BitTiming>& Database::bit_timing() const {
  return bit_timing_;
}

const std::map<std::string, std::unique_ptr<Node>>& Database::nodes() const {
  return nodes_;
}

const std::map<std::string, std::unique_ptr<ValueTable>>& Database::value_tables() const {
  return value_tables_;
}

const std::map<MessageId, std::unique_ptr<Message>>& Database::messages() const {
  return messages_;
}

const std::map<std::string, std::unique_ptr<EnvironmentVariable>>& Database::environment_variables() const {
  return env_vars_;
}

const std::map<std::string, std::unique_ptr<SignalType>>& Database::signal_types() const {
  return signal_types_;
}

const std::map<std::string, std::unique_ptr<AttributeDefinition>>& Database::attribute_definitions() const {
  return attribute_defs_;
}

void Database::set_version(const Version& version) {
  version_ = version;
}

void Database::set_bit_timing(const BitTiming& bit_timing) {
  bit_timing_ = bit_timing;
}

Node* Database::add_node(std::unique_ptr<Node> node) {
  const std::string& name = node->name();
  Node* node_ptr = node.get();
  nodes_[name] = std::move(node);
  return node_ptr;
}

Node* Database::get_node(const std::string& name) const {
  auto it = nodes_.find(name);
  if (it != nodes_.end()) {
    return it->second.get();
  }
  return nullptr;
}

ValueTable* Database::add_value_table(std::unique_ptr<ValueTable> table) {
  const std::string& name = table->name();
  ValueTable* table_ptr = table.get();
  value_tables_[name] = std::move(table);
  return table_ptr;
}

Message* Database::add_message(std::unique_ptr<Message> message) {
  MessageId id = message->id();
  Message* message_ptr = message.get();
  messages_[id] = std::move(message);
  return message_ptr;
}

Message* Database::get_message(MessageId id) const {
  auto it = messages_.find(id);
  if (it != messages_.end()) {
    return it->second.get();
  }
  return nullptr;
}

bool Database::remove_message(MessageId id) {
  return messages_.erase(id) > 0;
}

EnvironmentVariable* Database::add_env_var(std::unique_ptr<EnvironmentVariable> env_var) {
  const std::string& name = env_var->name();
  EnvironmentVariable* env_var_ptr = env_var.get();
  env_vars_[name] = std::move(env_var);
  return env_var_ptr;
}

SignalType* Database::add_signal_type(std::unique_ptr<SignalType> signal_type) {
  const std::string& name = signal_type->name();
  SignalType* signal_type_ptr = signal_type.get();
  signal_types_[name] = std::move(signal_type);
  return signal_type_ptr;
}

AttributeDefinition* Database::add_attribute_definition(std::unique_ptr<AttributeDefinition> def) {
  const std::string& name = def->name();
  AttributeDefinition* def_ptr = def.get();
  attribute_defs_[name] = std::move(def);
  return def_ptr;
}

std::map<std::string, double> Database::decode_message(MessageId id, const std::vector<uint8_t>& data) const {
  Message* message = get_message(id);
  if (message) {
    return message->decode(data);
  }
  return {};
}

} // namespace dbc_parser 