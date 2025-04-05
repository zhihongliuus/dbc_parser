#ifndef SRC_DBC_PARSER_PARSER_COMMON_TYPES_H_
#define SRC_DBC_PARSER_PARSER_COMMON_TYPES_H_

#include <map>
#include <optional>
#include <string>
#include <vector>
#include <variant>

namespace dbc_parser {
namespace parser {

// Common types used across different parsers

// Signal multiplexing type
enum class MultiplexType {
  kNone = 0,       // Not multiplexed
  kMultiplexor,    // This signal is the multiplexor
  kMultiplexed     // This signal is multiplexed
};

// Signal value sign type
enum class SignType {
  kUnsigned = 0,
  kSigned = 1
};

// Comment type enumeration
enum class CommentType {
  NETWORK = 0,
  NODE = 1,
  MESSAGE = 2,
  SIGNAL = 3,
  ENV_VAR = 4
};

// The object type to which an attribute applies
enum class AttributeObjectType {
  UNDEFINED,
  NETWORK,  // Global attribute for the entire network
  NODE,     // Applies to a node (BU_)
  MESSAGE,  // Applies to a message (BO_)
  SIGNAL,   // Applies to a signal (SG_)
  ENV_VAR   // Applies to an environment variable (EV_)
};

// Type of attribute value
enum class AttributeValueType {
  INT,      // Integer value
  HEX,      // Hexadecimal value (treated as INT in many operations)
  FLOAT,    // Floating point value
  STRING,   // String value
  ENUM      // Enumeration value
};

// Basic signal structure used across the parser
struct Signal {
  std::string name;                   // Signal name
  int start_bit = 0;                  // Start bit position
  int signal_size = 0;                // Size in bits (same as 'length')
  int length = 0;                     // Length in bits
  int byte_order = 1;                 // Byte order (1=little endian, 0=big endian)
  bool is_little_endian = true;       // Internal flag for byte order
  bool is_signed = false;             // Sign (+ or -)
  SignType sign = SignType::kUnsigned; // Sign type
  double factor = 1.0;                // Scaling factor
  double offset = 0.0;                // Offset
  double minimum = 0.0;               // Minimum value
  double maximum = 0.0;               // Maximum value
  std::string unit;                   // Unit (e.g., "km/h")
  std::vector<std::string> receivers; // Receiving nodes
  bool is_multiplexer = false;        // Whether signal is a multiplexer switch
  std::optional<int> multiplex_value; // Multiplexer value if signal is multiplexed
  MultiplexType multiplex_type = MultiplexType::kNone; // Multiplexing type
  int multiplex_value_int = -1;       // Integer representation of multiplex_value
};

// Basic message structure
struct Message {
  int id = 0;                 // Message ID
  std::string name;           // Message name
  int dlc = 0;                // Data Length Code (size in bytes)
  std::string sender;         // Sender node
  std::vector<Signal> signals;// Signals in the message
};

// Basic environment variable structure
struct EnvironmentVariable {
  std::string name;                    // Environment variable name
  int var_type = 0;                    // Variable type
  double minimum = 0.0;                // Minimum value
  double maximum = 0.0;                // Maximum value
  std::string unit;                    // Unit
  double initial_value = 0.0;          // Initial value
  int ev_id = 0;                       // Environment variable ID
  std::string access_type;             // Access type
  std::string access_nodes;            // Access nodes as string
};

// Environment variable data
struct EnvironmentVariableData {
  std::string name;                    // Environment variable name
  std::string data;                    // Environment variable data
};

// Comment structure
struct Comment {
  CommentType type;                    // Comment type
  std::variant<
    std::monostate,                   // No identifier (NETWORK)
    std::string,                      // Object name (NODE, ENV_VAR)
    int,                              // Object ID (MESSAGE)
    std::pair<int, std::string>       // Message ID and signal name (SIGNAL)
  > identifier;
  std::string text;                    // Comment text
};

// Node structure
struct Node {
  std::string name;                    // Node name
};

// Bit timing structure
struct BitTiming {
  int baudrate;                        // Baud rate in kbit/s
  double btr1_btr2;                    // Combined BTR1 and BTR2 register values
};

// Enum to identify the type of object that a value description is associated with
enum class ValueDescriptionType {
  SIGNAL,        // Signal value description (VAL_ <message id> <signal name>)
  ENV_VAR        // Environment variable value description (VAL_ <env var name>)
};

// Value table structure
struct ValueTable {
  std::string name;                    // Value table name
  std::map<int, std::string> values;   // Values mapping
};

// Helper to convert common parser types to DBC file types
class TypeConverter {
public:
  // Convert Signal to various DbcFile signal formats
  // Add methods as needed for conversions between equivalent types
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_COMMON_TYPES_H_ 