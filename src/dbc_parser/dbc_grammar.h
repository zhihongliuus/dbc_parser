#ifndef DBC_PARSER_DBC_GRAMMAR_H_
#define DBC_PARSER_DBC_GRAMMAR_H_

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <optional>
#include <variant>

#include "dbc_parser/types.h"

namespace dbc_parser {

// Forward declarations
class Database;
class Node;
class Message;
class Signal;

// Structure to store message data during parsing
struct MessageStruct {
  MessageId id;
  std::string name;
  uint32_t length;
  std::string sender;
};

// Structure to store signal data during parsing
struct SignalStruct {
  std::string name;
  uint32_t start_bit;
  uint32_t length;
  bool is_little_endian;
  bool is_signed;
  double factor;
  double offset;
  double min_value;
  double max_value;
  std::string unit;
  MultiplexerType mux_type = MultiplexerType::kNone;
  uint32_t mux_value = 0;
  std::vector<std::string> receivers;
};

// Structure to store value description data during parsing
struct ValueDescriptionStruct {
  MessageId msg_id;
  std::string signal_name;
  std::map<int, std::string> values;
};

// Structure to store attribute data during parsing
struct AttributeDefStruct {
  std::string name;
  AttributeType type;
  std::string object_type;
  std::variant<std::monostate, int, double, std::string> min_value;
  std::variant<std::monostate, int, double, std::string> max_value;
};

// Structure to store attribute default data during parsing
struct AttributeDefaultStruct {
  std::string name;
  std::variant<std::monostate, int, double, std::string> value;
};

// Structure to store attribute value data during parsing
struct AttributeValueStruct {
  std::string name;
  std::variant<std::monostate, int, double, std::string> value;
  std::string object_type;
  std::optional<std::variant<std::string, MessageId>> object_name;
  std::optional<std::string> signal_name;
};

// Structure to store comments during parsing
struct CommentStruct {
  CommentType type;
  std::variant<std::monostate, std::string, MessageId> id;
  std::optional<std::string> signal_name;
  std::string comment;
};

// Structure to store environment variable data during parsing
struct EnvVarStruct {
  std::string name;
  EnvVarType type;
  double min_value;
  double max_value;
  std::string unit;
  double initial_value;
  std::string id;
  std::optional<AccessType> access_type;
  std::vector<std::string> access_nodes;
};

// Structure to store environment variable data during parsing
struct EnvVarDataStruct {
  std::string name;
  std::vector<uint32_t> data;
};

// Function to parse a DBC file and return a Database object
std::unique_ptr<Database> parse_dbc(const std::string& content);
std::string write_dbc(const Database& db);

} // namespace dbc_parser

#endif // DBC_PARSER_DBC_GRAMMAR_H_ 