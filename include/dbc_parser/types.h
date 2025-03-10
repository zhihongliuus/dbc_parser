#ifndef DBC_PARSER_TYPES_H_
#define DBC_PARSER_TYPES_H_

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <cstdint>
#include <unordered_map>
#include <variant>

namespace dbc_parser {

// Forward declarations
class Node;
class Message;
class Signal;
class SignalGroup;
class ValueTable;
class EnvironmentVariable;
class SignalType;
class AttributeDefinition;

// Type definitions
using MessageId = uint32_t;

// Enumerations
enum class MultiplexerType {
  kNone,
  kMultiplexor,
  kMultiplexed
};

enum class SignalExtendedValueType {
  kNone,
  kFloat,
  kDouble
};

enum class AttributeType {
  kInt,
  kFloat,
  kString,
  kEnum
};

enum class EnvVarType {
  kInteger,
  kFloat,
  kString
};

enum class EnvVarAccessType {
  kUnrestricted,
  kRead,
  kWrite,
  kReadWrite
};

// Database class to represent a DBC file
class Database {
public:
  Database() = default;
  ~Database() = default;
  
  // Delete copy constructor and assignment operator
  Database(const Database&) = delete;
  Database& operator=(const Database&) = delete;

  struct Version {
    std::string version;
  };

  struct BitTiming {
    uint32_t baudrate = 0;
    uint32_t btr1 = 0;
    uint32_t btr2 = 0;
  };

  // Accessors for version
  void set_version(const Version& version) { version_ = version; }
  const Version* version() const { return &version_; }

  // Accessors for bit timing
  void set_bit_timing(const BitTiming& bit_timing) { bit_timing_ = bit_timing; }
  const BitTiming* bit_timing() const { return &bit_timing_; }

  // Node management
  void add_node(std::unique_ptr<Node> node);
  const std::vector<std::unique_ptr<Node>>& nodes() const { return nodes_; }
  Node* get_node(const std::string& name) const;
  void set_node_comment(const std::string& node_name, const std::string& comment);

  // Message management
  void add_message(std::unique_ptr<Message> message);
  const std::vector<std::unique_ptr<Message>>& messages() const { return messages_; }
  Message* get_message(MessageId id) const;
  
  // New symbols management
  void set_new_symbols(const std::vector<std::string>& symbols) { new_symbols_ = symbols; }
  const std::vector<std::string>& new_symbols() const { return new_symbols_; }
  
  // Value table management
  void add_value_table(std::unique_ptr<ValueTable> value_table) {
    value_tables_.push_back(std::move(value_table));
  }
  
  const std::vector<std::unique_ptr<ValueTable>>& value_tables() const {
    return value_tables_;
  }
  
  ValueTable* get_value_table(const std::string& name) const;
  
  // Environment variable management
  void add_environment_variable(std::unique_ptr<EnvironmentVariable> env_var) {
    environment_variables_.push_back(std::move(env_var));
  }
  
  const std::vector<std::unique_ptr<EnvironmentVariable>>& environment_variables() const {
    return environment_variables_;
  }
  
  EnvironmentVariable* get_environment_variable(const std::string& name) const;
  
  // Signal type management
  void add_signal_type(std::unique_ptr<SignalType> signal_type) {
    signal_types_.push_back(std::move(signal_type));
  }
  
  const std::vector<std::unique_ptr<SignalType>>& signal_types() const {
    return signal_types_;
  }
  
  SignalType* get_signal_type(const std::string& name) const;
  
  // Attribute definition management
  void add_attribute_definition(std::unique_ptr<AttributeDefinition> attr_def) {
    attribute_definitions_.push_back(std::move(attr_def));
  }
  
  const std::vector<std::unique_ptr<AttributeDefinition>>& attribute_definitions() const {
    return attribute_definitions_;
  }
  
  AttributeDefinition* get_attribute_definition(const std::string& name) const;
  
  // Attribute default values
  void set_attribute_default(const std::string& attr_name, 
                            const std::variant<int, double, std::string>& value) {
    attribute_defaults_[attr_name] = value;
  }
  
  const std::map<std::string, std::variant<int, double, std::string>>& 
  attribute_defaults() const {
    return attribute_defaults_;
  }
  
  // Global attribute values
  void set_global_attribute(const std::string& attr_name, 
                           const std::variant<int, double, std::string>& value) {
    global_attributes_[attr_name] = value;
  }
  
  const std::map<std::string, std::variant<int, double, std::string>>& 
  global_attributes() const {
    return global_attributes_;
  }
  
  // Node attribute values
  void set_node_attribute(const std::string& node_name, const std::string& attr_name,
                         const std::variant<int, double, std::string>& value) {
    node_attributes_[node_name][attr_name] = value;
  }
  
  const std::map<std::string, std::map<std::string, std::variant<int, double, std::string>>>& 
  node_attributes() const {
    return node_attributes_;
  }
  
  // Message attribute values
  void set_message_attribute(MessageId msg_id, const std::string& attr_name,
                            const std::variant<int, double, std::string>& value) {
    message_attributes_[msg_id][attr_name] = value;
  }
  
  const std::map<MessageId, std::map<std::string, std::variant<int, double, std::string>>>& 
  message_attributes() const {
    return message_attributes_;
  }
  
  // Signal attribute values
  void set_signal_attribute(MessageId msg_id, const std::string& signal_name, 
                           const std::string& attr_name,
                           const std::variant<int, double, std::string>& value) {
    signal_attributes_[msg_id][signal_name][attr_name] = value;
  }
  
  const std::map<MessageId, 
        std::map<std::string, 
            std::map<std::string, std::variant<int, double, std::string>>>>& 
  signal_attributes() const {
    return signal_attributes_;
  }
  
  // Signal type references
  void set_signal_type_ref(MessageId msg_id, const std::string& signal_name,
                          const std::string& type_ref) {
    signal_type_refs_[msg_id][signal_name] = type_ref;
  }
  
  const std::map<MessageId, std::map<std::string, std::string>>& 
  signal_type_refs() const {
    return signal_type_refs_;
  }

private:
  Version version_;
  BitTiming bit_timing_;
  std::vector<std::string> new_symbols_;
  std::vector<std::unique_ptr<Node>> nodes_;
  std::vector<std::unique_ptr<Message>> messages_;
  std::map<std::string, std::string> node_comments_;
  std::vector<std::unique_ptr<ValueTable>> value_tables_;
  std::vector<std::unique_ptr<EnvironmentVariable>> environment_variables_;
  std::vector<std::unique_ptr<SignalType>> signal_types_;
  std::vector<std::unique_ptr<AttributeDefinition>> attribute_definitions_;
  std::map<std::string, std::variant<int, double, std::string>> attribute_defaults_;
  std::map<std::string, std::variant<int, double, std::string>> global_attributes_;
  std::map<std::string, std::map<std::string, std::variant<int, double, std::string>>> node_attributes_;
  std::map<MessageId, std::map<std::string, std::variant<int, double, std::string>>> message_attributes_;
  std::map<MessageId, 
        std::map<std::string, 
            std::map<std::string, std::variant<int, double, std::string>>>> signal_attributes_;
  std::map<MessageId, std::map<std::string, std::string>> signal_type_refs_;
};

// Node class for ECU/node definitions
class Node {
public:
  explicit Node(const std::string& name) : name_(name) {}
  ~Node() = default;

  const std::string& name() const { return name_; }
  void set_comment(const std::string& comment) { comment_ = comment; }
  const std::string& comment() const { return comment_; }
  
  // Node attributes
  void set_attribute(const std::string& attr_name, 
                    const std::variant<int, double, std::string>& value) {
    attributes_[attr_name] = value;
  }
  
  const std::map<std::string, std::variant<int, double, std::string>>& 
  attributes() const {
    return attributes_;
  }

private:
  std::string name_;
  std::string comment_;
  std::map<std::string, std::variant<int, double, std::string>> attributes_;
};

// Message class for CAN message definitions
class Message {
public:
  Message(MessageId id, const std::string& name, uint32_t length, const std::string& sender)
    : id_(id), name_(name), length_(length), sender_(sender) {}
  ~Message() = default;

  MessageId id() const { return id_; }
  const std::string& name() const { return name_; }
  uint32_t length() const { return length_; }
  const std::string& sender() const { return sender_; }

  // Signal management
  void add_signal(std::unique_ptr<Signal> signal);
  const std::unordered_map<std::string, std::unique_ptr<Signal>>& signals() const { return signals_; }
  Signal* get_signal(const std::string& name) const;

  // Comment
  void set_comment(const std::string& comment) { comment_ = comment; }
  const std::string& comment() const { return comment_; }

  // Transmitters
  void add_transmitter(const std::string& transmitter) { transmitters_.push_back(transmitter); }
  const std::vector<std::string>& transmitters() const { return transmitters_; }

  // Signal groups
  void add_signal_group(std::unique_ptr<SignalGroup> group);
  const std::vector<std::unique_ptr<SignalGroup>>& signal_groups() const { return signal_groups_; }
  
  // Message attributes
  void set_attribute(const std::string& attr_name, 
                    const std::variant<int, double, std::string>& value) {
    attributes_[attr_name] = value;
  }
  
  const std::map<std::string, std::variant<int, double, std::string>>& 
  attributes() const {
    return attributes_;
  }

private:
  MessageId id_;
  std::string name_;
  uint32_t length_;
  std::string sender_;
  std::unordered_map<std::string, std::unique_ptr<Signal>> signals_;
  std::string comment_;
  std::vector<std::string> transmitters_;
  std::vector<std::unique_ptr<SignalGroup>> signal_groups_;
  std::map<std::string, std::variant<int, double, std::string>> attributes_;
};

// Signal class for CAN signal definitions
class Signal {
public:
  Signal(const std::string& name, uint32_t start_bit, uint32_t length,
         bool is_little_endian, bool is_signed, double factor, double offset,
         double min_value, double max_value, const std::string& unit)
    : name_(name), start_bit_(start_bit), length_(length),
      is_little_endian_(is_little_endian), is_signed_(is_signed),
      factor_(factor), offset_(offset), min_value_(min_value), max_value_(max_value),
      unit_(unit), mux_type_(MultiplexerType::kNone), mux_value_(0),
      extended_value_type_(SignalExtendedValueType::kNone) {}
  ~Signal() = default;

  const std::string& name() const { return name_; }
  uint32_t start_bit() const { return start_bit_; }
  uint32_t length() const { return length_; }
  bool is_little_endian() const { return is_little_endian_; }
  bool is_signed() const { return is_signed_; }
  double factor() const { return factor_; }
  double offset() const { return offset_; }
  double min_value() const { return min_value_; }
  double max_value() const { return max_value_; }
  const std::string& unit() const { return unit_; }

  // Receivers
  void add_receiver(const std::string& receiver) { receivers_.push_back(receiver); }
  const std::vector<std::string>& receivers() const { return receivers_; }

  // Multiplexing
  void set_mux_type(MultiplexerType type) { mux_type_ = type; }
  MultiplexerType mux_type() const { return mux_type_; }

  void set_mux_value(uint32_t value) { mux_value_ = value; }
  uint32_t mux_value() const { return mux_value_; }

  // Value descriptions (enum values)
  void add_value_description(int64_t value, const std::string& description) {
    value_descriptions_[value] = description;
  }
  const std::map<int64_t, std::string>& value_descriptions() const { return value_descriptions_; }

  // Comment
  void set_comment(const std::string& comment) { comment_ = comment; }
  const std::string& comment() const { return comment_; }

  // Extended value type
  void set_extended_value_type(SignalExtendedValueType type) { extended_value_type_ = type; }
  SignalExtendedValueType extended_value_type() const { return extended_value_type_; }
  
  // Signal type reference
  void set_type_reference(const std::string& type_ref) { type_reference_ = type_ref; }
  const std::string& type_reference() const { return type_reference_; }
  
  // Signal attributes
  void set_attribute(const std::string& attr_name, 
                    const std::variant<int, double, std::string>& value) {
    attributes_[attr_name] = value;
  }
  
  const std::map<std::string, std::variant<int, double, std::string>>& 
  attributes() const {
    return attributes_;
  }

private:
  std::string name_;
  uint32_t start_bit_;
  uint32_t length_;
  bool is_little_endian_;
  bool is_signed_;
  double factor_;
  double offset_;
  double min_value_;
  double max_value_;
  std::string unit_;
  std::vector<std::string> receivers_;
  MultiplexerType mux_type_;
  uint32_t mux_value_;
  std::map<int64_t, std::string> value_descriptions_;
  std::string comment_;
  SignalExtendedValueType extended_value_type_;
  std::string type_reference_; // Reference to a signal type
  std::map<std::string, std::variant<int, double, std::string>> attributes_; // Signal attributes
};

// SignalGroup class for signal group definitions
class SignalGroup {
public:
  SignalGroup(MessageId message_id, const std::string& name, uint32_t id)
    : message_id_(message_id), name_(name), id_(id) {}
  ~SignalGroup() = default;

  MessageId message_id() const { return message_id_; }
  const std::string& name() const { return name_; }
  uint32_t id() const { return id_; }

  void add_signal(const std::string& signal_name) { signals_.push_back(signal_name); }
  const std::vector<std::string>& signals() const { return signals_; }

private:
  MessageId message_id_;
  std::string name_;
  uint32_t id_;
  std::vector<std::string> signals_;
};

// Value table class to represent VAL_TABLE_ entries
class ValueTable {
public:
  explicit ValueTable(const std::string& name) : name_(name) {}
  
  const std::string& name() const { return name_; }
  
  void add_value(int64_t value, const std::string& description) {
    values_[value] = description;
  }
  
  const std::map<int64_t, std::string>& values() const { return values_; }

private:
  std::string name_;
  std::map<int64_t, std::string> values_;
};

// Signal Type class for signal type definitions
class SignalType {
public:
  SignalType(const std::string& name, double min_value, double max_value, 
             const std::string& unit, double factor, double offset, 
             uint32_t length, bool is_signed)
    : name_(name), min_value_(min_value), max_value_(max_value),
      unit_(unit), factor_(factor), offset_(offset),
      length_(length), is_signed_(is_signed) {}

  const std::string& name() const { return name_; }
  double min_value() const { return min_value_; }
  double max_value() const { return max_value_; }
  const std::string& unit() const { return unit_; }
  double factor() const { return factor_; }
  double offset() const { return offset_; }
  uint32_t length() const { return length_; }
  bool is_signed() const { return is_signed_; }

  // Value table reference
  void set_value_table(const std::string& value_table) { value_table_ = value_table; }
  const std::string& value_table() const { return value_table_; }

private:
  std::string name_;
  double min_value_;
  double max_value_;
  std::string unit_;
  double factor_;
  double offset_;
  uint32_t length_;
  bool is_signed_;
  std::string value_table_; // Reference to a value table
};

// Attribute-related classes
class AttributeDefinition {
public:
  AttributeDefinition(const std::string& name, AttributeType type)
    : name_(name), type_(type) {}

  const std::string& name() const { return name_; }
  AttributeType type() const { return type_; }
  
  void set_min(const std::variant<int, double, std::string>& min) { min_ = min; }
  void set_max(const std::variant<int, double, std::string>& max) { max_ = max; }
  
  const std::variant<int, double, std::string>& min() const { return min_; }
  const std::variant<int, double, std::string>& max() const { return max_; }
  
  void add_enum_value(int value, const std::string& description) {
    enum_values_[value] = description;
  }
  
  const std::map<int, std::string>& enum_values() const { return enum_values_; }

private:
  std::string name_;
  AttributeType type_;
  std::variant<int, double, std::string> min_;
  std::variant<int, double, std::string> max_;
  std::map<int, std::string> enum_values_; // For enum type attributes
};

// Environment variable class
class EnvironmentVariable {
public:
  EnvironmentVariable(const std::string& name, EnvVarType type, 
                     double min_value, double max_value, 
                     const std::string& unit, double initial_value,
                     uint32_t ev_id, EnvVarAccessType access_type, 
                     const std::vector<std::string>& access_nodes)
    : name_(name), type_(type), min_value_(min_value), max_value_(max_value),
      unit_(unit), initial_value_(initial_value), ev_id_(ev_id),
      access_type_(access_type), access_nodes_(access_nodes) {}
  
  const std::string& name() const { return name_; }
  EnvVarType type() const { return type_; }
  double min_value() const { return min_value_; }
  double max_value() const { return max_value_; }
  const std::string& unit() const { return unit_; }
  double initial_value() const { return initial_value_; }
  uint32_t ev_id() const { return ev_id_; }
  EnvVarAccessType access_type() const { return access_type_; }
  const std::vector<std::string>& access_nodes() const { return access_nodes_; }
  
  // Environment variable data list
  void add_data_value(uint64_t value, const std::string& description) {
    data_values_[value] = description;
  }
  
  const std::map<uint64_t, std::string>& data_values() const { return data_values_; }
  
  // Comment
  void set_comment(const std::string& comment) { comment_ = comment; }
  const std::string& comment() const { return comment_; }

private:
  std::string name_;
  EnvVarType type_;
  double min_value_;
  double max_value_;
  std::string unit_;
  double initial_value_;
  uint32_t ev_id_;
  EnvVarAccessType access_type_;
  std::vector<std::string> access_nodes_;
  std::map<uint64_t, std::string> data_values_; // Environment variable data
  std::string comment_;
};

// Implement Database methods
inline void Database::add_node(std::unique_ptr<Node> node) {
  nodes_.push_back(std::move(node));
}

inline Node* Database::get_node(const std::string& name) const {
  for (const auto& node : nodes_) {
    if (node->name() == name) {
      return node.get();
    }
  }
  return nullptr;
}

inline void Database::set_node_comment(const std::string& node_name, const std::string& comment) {
  node_comments_[node_name] = comment;
  auto node = get_node(node_name);
  if (node) {
    node->set_comment(comment);
  }
}

inline void Database::add_message(std::unique_ptr<Message> message) {
  messages_.push_back(std::move(message));
}

inline Message* Database::get_message(MessageId id) const {
  for (const auto& message : messages_) {
    if (message->id() == id) {
      return message.get();
    }
  }
  return nullptr;
}

inline ValueTable* Database::get_value_table(const std::string& name) const {
  for (const auto& table : value_tables_) {
    if (table->name() == name) {
      return table.get();
    }
  }
  return nullptr;
}

inline EnvironmentVariable* Database::get_environment_variable(const std::string& name) const {
  for (const auto& env_var : environment_variables_) {
    if (env_var->name() == name) {
      return env_var.get();
    }
  }
  return nullptr;
}

inline SignalType* Database::get_signal_type(const std::string& name) const {
  for (const auto& signal_type : signal_types_) {
    if (signal_type->name() == name) {
      return signal_type.get();
    }
  }
  return nullptr;
}

inline AttributeDefinition* Database::get_attribute_definition(const std::string& name) const {
  for (const auto& attr_def : attribute_definitions_) {
    if (attr_def->name() == name) {
      return attr_def.get();
    }
  }
  return nullptr;
}

// Implement Message methods
inline void Message::add_signal(std::unique_ptr<Signal> signal) {
  std::string name = signal->name();
  signals_[name] = std::move(signal);
}

inline Signal* Message::get_signal(const std::string& name) const {
  auto it = signals_.find(name);
  if (it != signals_.end()) {
    return it->second.get();
  }
  return nullptr;
}

inline void Message::add_signal_group(std::unique_ptr<SignalGroup> group) {
  signal_groups_.push_back(std::move(group));
}

} // namespace dbc_parser

#endif // DBC_PARSER_TYPES_H_
