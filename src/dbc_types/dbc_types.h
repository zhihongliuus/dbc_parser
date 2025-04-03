#ifndef DBC_PARSER_SRC_DBC_TYPES_DBC_TYPES_H_
#define DBC_PARSER_SRC_DBC_TYPES_DBC_TYPES_H_

#include <string>
#include <vector>
#include <unordered_map>

namespace dbc_parser {

// Structure for version information
struct Version {
  std::string version_string;
};

// Structure for a node (ECU)
struct Node {
  std::string name;
  std::string description;
};

// Enum for signal type
enum class SignalType {
  kInteger,
  kFloat,
  kDouble
};

// Enum for byte order
enum class ByteOrder {
  kLittleEndian,
  kBigEndian
};

// Structure for a signal
struct Signal {
  std::string name;
  std::string description;
  int start_bit;
  int length;
  ByteOrder byte_order;
  bool is_signed;
  double factor;
  double offset;
  double min_value;
  double max_value;
  std::string unit;
  std::string receiver_nodes;
  bool is_multiplexer;
  int multiplexer_value;
  SignalType signal_type;
  std::unordered_map<int, std::string> value_descriptions;
};

// Structure for a message
struct Message {
  int id;
  std::string name;
  int dlc;  // Data Length Code
  std::string sender;
  std::string description;
  std::vector<Signal> signals;
  std::unordered_map<std::string, std::vector<Signal>> multiplexed_signals;
};

// Structure for attribute definition
struct AttributeDefinition {
  std::string name;
  std::string type;  // "INT", "HEX", "FLOAT", "STRING", "ENUM"
  std::string default_value;
  std::string min_value;
  std::string max_value;
  std::vector<std::string> enum_values;
};

// Structure for attribute value
struct AttributeValue {
  std::string attribute_name;
  std::string value;
};

// Structure for environment variable
struct EnvironmentVariable {
  std::string name;
  int type;
  double min_value;
  double max_value;
  std::string unit;
  double initial_value;
  int ev_id;
  std::string access_type;
  std::vector<std::string> access_nodes;
};

// Structure for value table
struct ValueTable {
  std::string name;
  std::unordered_map<int, std::string> value_descriptions;
};

// Structure for signal group
struct SignalGroup {
  std::string name;
  int message_id;
  int repetitions;
  std::vector<std::string> signal_names;
};

}  // namespace dbc_parser

#endif  // DBC_PARSER_SRC_DBC_TYPES_DBC_TYPES_H_ 