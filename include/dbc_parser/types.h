#ifndef DBC_PARSER_TYPES_H_
#define DBC_PARSER_TYPES_H_

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <cstdint>
#include <unordered_map>

namespace dbc_parser {

// Forward declarations
class Node;
class Message;
class Signal;
class SignalGroup;

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

private:
  Version version_;
  BitTiming bit_timing_;
  std::vector<std::unique_ptr<Node>> nodes_;
  std::vector<std::unique_ptr<Message>> messages_;
  std::map<std::string, std::string> node_comments_;
};

// Node class for ECU/node definitions
class Node {
public:
  explicit Node(const std::string& name) : name_(name) {}
  ~Node() = default;

  const std::string& name() const { return name_; }
  void set_comment(const std::string& comment) { comment_ = comment; }
  const std::string& comment() const { return comment_; }

private:
  std::string name_;
  std::string comment_;
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

private:
  MessageId id_;
  std::string name_;
  uint32_t length_;
  std::string sender_;
  std::unordered_map<std::string, std::unique_ptr<Signal>> signals_;
  std::string comment_;
  std::vector<std::string> transmitters_;
  std::vector<std::unique_ptr<SignalGroup>> signal_groups_;
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
