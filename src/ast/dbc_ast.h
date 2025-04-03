#ifndef SRC_AST_DBC_AST_H_
#define SRC_AST_DBC_AST_H_

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory> // Include for std::unique_ptr

namespace dbc_parser {
namespace ast {

// Enum for byte order
enum class ByteOrder {
  kLittleEndian,
  kBigEndian
};

// Structure for a Node (ECU)
struct Node {
  std::string name;
  std::string comment;
  std::map<std::string, std::string> attributes;
};

// Structure for a Signal
struct Signal {
  std::string name;
  int start_bit = 0;
  int length = 0;
  ByteOrder byte_order = ByteOrder::kLittleEndian;
  bool is_signed = false;
  double factor = 1.0;
  double offset = 0.0;
  double min_value = 0.0;
  double max_value = 0.0;
  std::string unit;
  std::vector<std::string> receiver_nodes;
  std::string comment;
  std::unordered_map<int, std::string> value_descriptions;

  // Multiplexing support
  bool is_multiplexer = false;       // True if this signal is a multiplexer
  int multiplexer_value = -1;        // Multiplexer value for multiplexed signals, -1 if not multiplexed signal itself

  // Attributes
  std::map<std::string, std::string> attributes;
};

// Structure for a Message
struct Message {
  uint32_t id = 0; // Changed to uint32_t to accommodate extended IDs potentially
  std::string name;
  int dlc = 0;  // Data Length Code
  std::string sender;
  std::string comment;
  std::vector<Signal> signals;

  // Attributes
  std::map<std::string, std::string> attributes;
};

// Structure for Value Table
struct ValueTable {
  std::string name;
  std::unordered_map<int, std::string> value_descriptions;
};

// Structure for Signal Group
struct SignalGroup {
  std::string name;
  uint32_t message_id; // Changed to uint32_t
  int repetitions;
  std::vector<std::string> signal_names;
};

// Structure for Attribute Definition
struct AttributeDefinition {
  std::string name;
  std::string object_type; // BU_, BO_, SG_, "" (global) - Optional
  std::string type;        // "INT", "HEX", "FLOAT", "STRING", "ENUM"
  std::string default_value; // Can be numeric or string based on type
  double min_value = 0.0;  // Applicable for INT, HEX, FLOAT
  double max_value = 0.0;  // Applicable for INT, HEX, FLOAT
  std::vector<std::string> enum_values; // Applicable for ENUM
};

// Class representing a parsed DBC file (The root of the AST)
class DbcFile {
 public:
  DbcFile() = default;
  ~DbcFile() = default;

  // Disable copy and move operations for simplicity for now
  DbcFile(const DbcFile&) = delete;
  DbcFile& operator=(const DbcFile&) = delete;
  DbcFile(DbcFile&&) = delete;
  DbcFile& operator=(DbcFile&&) = delete;

  // Version information
  std::string version;

  // Nodes (ECUs)
  std::vector<Node> nodes;

  // Messages
  std::vector<Message> messages;

  // Value Tables
  std::vector<ValueTable> value_tables;

  // Signal Groups
  std::vector<SignalGroup> signal_groups;

  // Attribute Definitions
  std::vector<AttributeDefinition> attribute_definitions;

  // Attribute Values (applied attributes) - Simplified for now
  // A more structured approach might map object -> attribute -> value
  // For now, storing them within the respective objects (Node, Message, Signal)

  // --- Helper methods to find elements ---
  Node* FindNode(const std::string& name);
  Message* FindMessage(uint32_t id);
  Message* FindMessage(const std::string& name);
  Signal* FindSignal(uint32_t message_id, const std::string& signal_name);
  AttributeDefinition* FindAttributeDefinition(const std::string& name);

};

// --- Implementation of helper methods ---

inline Node* DbcFile::FindNode(const std::string& name) {
    for (auto& node : nodes) {
        if (node.name == name) {
            return &node;
        }
    }
    return nullptr;
}

inline Message* DbcFile::FindMessage(uint32_t id) {
    for (auto& msg : messages) {
        if (msg.id == id) {
            return &msg;
        }
    }
    return nullptr;
}

inline Message* DbcFile::FindMessage(const std::string& name) {
    for (auto& msg : messages) {
        if (msg.name == name) {
            return &msg;
        }
    }
    return nullptr;
}

inline Signal* DbcFile::FindSignal(uint32_t message_id, const std::string& signal_name) {
    Message* msg = FindMessage(message_id);
    if (msg) {
        for (auto& sig : msg->signals) {
            if (sig.name == signal_name) {
                return &sig;
            }
        }
    }
    return nullptr;
}

inline AttributeDefinition* DbcFile::FindAttributeDefinition(const std::string& name) {
    for (auto& attr_def : attribute_definitions) {
        if (attr_def.name == name) {
            return &attr_def;
        }
    }
    return nullptr;
}


} // namespace ast
} // namespace dbc_parser

#endif // SRC_AST_DBC_AST_H_ 