#ifndef DBC_PARSER_TYPES_H_
#define DBC_PARSER_TYPES_H_

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace dbc_parser {

// Basic type definitions
using MessageId = uint32_t;
using SignalId = uint32_t;
using NodeId = uint32_t;
using AttributeId = uint32_t;

// Forward declarations
class Signal;
class Message;
class Node;
class Database;
class ValueTable;
class Attribute;
class AttributeDefinition;
class EnvironmentVariable;
class SignalType;
class SignalGroup;

// Multiplexer types
enum class MultiplexerType {
  kNone,
  kMultiplexor,
  kMultiplexed
};

// Signal extended value type
enum class SignalExtendedValueType {
  kNone,
  kInteger,
  kFloat,
  kDouble
};

// Attribute type
enum class AttributeType {
  kInteger,
  kFloat,
  kString,
  kEnum
};

// Signal class
class Signal {
 public:
  Signal(const std::string& name, uint32_t start_bit, uint32_t length,
         bool is_little_endian, bool is_signed, double factor, double offset,
         double min_value, double max_value, const std::string& unit);
  
  // Getters
  const std::string& name() const;
  uint32_t start_bit() const;
  uint32_t length() const;
  bool is_little_endian() const;
  bool is_signed() const;
  double factor() const;
  double offset() const;
  double min_value() const;
  double max_value() const;
  const std::string& unit() const;
  const std::string& comment() const;
  MultiplexerType mux_type() const;
  uint32_t mux_value() const;
  const std::vector<std::string>& receivers() const;
  const std::map<uint64_t, std::string>& value_descriptions() const;
  
  // Setters
  void set_comment(const std::string& comment);
  void set_mux_type(MultiplexerType type);
  void set_mux_value(uint32_t value);
  void set_factor(double factor);
  void set_offset(double offset);
  
  // Modifiers
  void add_receiver(const std::string& receiver);
  void add_value_description(uint64_t value, const std::string& description);
  void set_attribute(const std::string& name, const Attribute& attribute);
  
  // Signal operations
  double decode(const std::vector<uint8_t>& data) const;
  void encode(double value, std::vector<uint8_t>& data) const;
  
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
  std::string comment_;
  MultiplexerType mux_type_;
  uint32_t mux_value_;
  std::vector<std::string> receivers_;
  std::map<uint64_t, std::string> value_descriptions_;
  std::map<std::string, Attribute> attributes_;
};

// Message class
class Message {
 public:
  Message(MessageId id, const std::string& name, uint32_t length, const std::string& sender);
  
  // Copy constructor to handle unique_ptr members
  Message(const Message& other);
  
  // Getters
  MessageId id() const;
  const std::string& name() const;
  uint32_t length() const;
  const std::string& sender() const;
  const std::string& comment() const;
  const std::map<std::string, std::unique_ptr<Signal>>& signals() const;
  const std::vector<std::string>& transmitters() const;
  
  // Setters
  void set_comment(const std::string& comment);
  
  // Modifiers
  Signal* add_signal(std::unique_ptr<Signal> signal);
  Signal* get_signal(const std::string& name) const;
  bool remove_signal(const std::string& name);
  void set_attribute(const std::string& name, const Attribute& attribute);
  void add_transmitter(const std::string& transmitter);
  void add_signal_group(std::unique_ptr<SignalGroup> group);
  
  // Message operations
  std::map<std::string, double> decode(const std::vector<uint8_t>& data) const;
 
 private:
  MessageId id_;
  std::string name_;
  uint32_t length_;
  std::string sender_;
  std::string comment_;
  std::map<std::string, std::unique_ptr<Signal>> signals_;
  std::vector<std::string> transmitters_;
  std::map<std::string, std::unique_ptr<SignalGroup>> signal_groups_;
  std::map<std::string, Attribute> attributes_;
};

// Node class
class Node {
 public:
  explicit Node(const std::string& name);
  
  // Getters
  const std::string& name() const;
  const std::string& comment() const;
  
  // Setters
  void set_comment(const std::string& comment);
  
  // Modifiers
  void set_attribute(const std::string& name, const Attribute& attribute);

 private:
  std::string name_;
  std::string comment_;
  std::map<std::string, Attribute> attributes_;
};

// Value table class
class ValueTable {
 public:
  explicit ValueTable(const std::string& name);
  
  // Getters
  const std::string& name() const;
  const std::map<uint64_t, std::string>& entries() const;
  
  // Modifiers
  void add_entry(uint64_t value, const std::string& description);

 private:
  std::string name_;
  std::map<uint64_t, std::string> entries_;
};

// Attribute class
class Attribute {
 public:
  Attribute() = default;
  Attribute(const std::string& name, const std::string& value);
  
  // Getters
  const std::string& name() const;
  const std::string& value() const;
  
  // Setters
  void set_value(const std::string& value);

 private:
  std::string name_;
  std::string value_;
};

// Signal Group class
class SignalGroup {
 public:
  SignalGroup(const std::string& name, uint32_t id);
  
  // Getters
  const std::string& name() const;
  uint32_t id() const;
  
  // Modifiers
  void add_signal(const std::string& signal_name);

 private:
  std::string name_;
  uint32_t id_;
  std::vector<std::string> signals_;
};

// Environment Variable class
class EnvironmentVariable {
 public:
  EnvironmentVariable(const std::string& name, uint32_t type, double min, double max);
  
  // Getters
  const std::string& name() const;
  uint32_t type() const;
  double min() const;
  double max() const;

 private:
  std::string name_;
  uint32_t type_;
  double min_;
  double max_;
};

// Signal Type class
class SignalType {
 public:
  SignalType(const std::string& name);
  
  // Getters
  const std::string& name() const;

 private:
  std::string name_;
};

// Attribute Definition class
class AttributeDefinition {
 public:
  enum class Type {
    kInteger,
    kFloat,
    kString,
    kEnum
  };
  
  AttributeDefinition(const std::string& name, Type type);
  
  // Getters
  const std::string& name() const;
  Type type() const;

 private:
  std::string name_;
  Type type_;
};

// Database class
class Database {
 public:
  // Version information
  struct Version {
    std::string version;
  };
  
  // Bit timing information
  struct BitTiming {
    uint32_t baudrate;
    uint32_t btr1;
    uint32_t btr2;
  };
  
  Database();
  Database(const Database& other);
  
  // Getters
  const std::optional<Version>& version() const;
  const std::optional<BitTiming>& bit_timing() const;
  const std::map<std::string, std::unique_ptr<Node>>& nodes() const;
  const std::map<std::string, std::unique_ptr<ValueTable>>& value_tables() const;
  const std::map<MessageId, std::unique_ptr<Message>>& messages() const;
  const std::map<std::string, std::unique_ptr<EnvironmentVariable>>& environment_variables() const;
  const std::map<std::string, std::unique_ptr<SignalType>>& signal_types() const;
  const std::map<std::string, std::unique_ptr<AttributeDefinition>>& attribute_definitions() const;
  
  // Setters
  void set_version(const Version& version);
  void set_bit_timing(const BitTiming& bit_timing);
  
  // Modifiers
  Node* add_node(std::unique_ptr<Node> node);
  Node* get_node(const std::string& name) const;
  ValueTable* add_value_table(std::unique_ptr<ValueTable> table);
  Message* add_message(std::unique_ptr<Message> message);
  Message* get_message(MessageId id) const;
  bool remove_message(MessageId id);
  EnvironmentVariable* add_env_var(std::unique_ptr<EnvironmentVariable> env_var);
  SignalType* add_signal_type(std::unique_ptr<SignalType> signal_type);
  AttributeDefinition* add_attribute_definition(std::unique_ptr<AttributeDefinition> def);
  
  // Database operations
  std::map<std::string, double> decode_message(MessageId id, const std::vector<uint8_t>& data) const;

 private:
  std::optional<Version> version_;
  std::optional<BitTiming> bit_timing_;
  std::map<std::string, std::unique_ptr<Node>> nodes_;
  std::map<std::string, std::unique_ptr<ValueTable>> value_tables_;
  std::map<MessageId, std::unique_ptr<Message>> messages_;
  std::map<std::string, std::unique_ptr<EnvironmentVariable>> env_vars_;
  std::map<std::string, std::unique_ptr<SignalType>> signal_types_;
  std::map<std::string, std::unique_ptr<AttributeDefinition>> attribute_defs_;
};

} // namespace dbc_parser

#endif // DBC_PARSER_TYPES_H_ 