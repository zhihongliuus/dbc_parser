#ifndef DBC_PARSER_PARSER_COMMON_TYPES_H_
#define DBC_PARSER_PARSER_COMMON_TYPES_H_

#include <map>
#include <optional>
#include <string>
#include <vector>
#include <variant>

namespace dbc_parser {
namespace parser {

/**
 * @brief Common types used across different DBC file parsers.
 *
 * This file defines all the common types used throughout the DBC parser library.
 * These include enumerations for different object types and data structures to
 * hold parsed information.
 */

/**
 * @brief Signal multiplexing type enumeration.
 *
 * Defines the multiplexing mode for a CAN signal.
 */
enum class MultiplexType {
  kNone = 0,       ///< Not multiplexed (regular signal)
  kMultiplexor,    ///< This signal is the multiplexor (selector)
  kMultiplexed     ///< This signal is multiplexed (depends on multiplexor value)
};

/**
 * @brief Signal value sign type enumeration.
 *
 * Defines whether a signal's value should be interpreted as signed or unsigned.
 */
enum class SignType {
  kUnsigned = 0,   ///< Unsigned value (≥0)
  kSigned = 1      ///< Signed value (can be negative)
};

/**
 * @brief Comment type enumeration.
 *
 * Defines the different types of comments that can appear in a DBC file.
 */
enum class CommentType {
  NETWORK = 0,     ///< Comment for the entire network
  NODE = 1,        ///< Comment for a specific node (ECU)
  MESSAGE = 2,     ///< Comment for a specific message
  SIGNAL = 3,      ///< Comment for a specific signal within a message
  ENV_VAR = 4      ///< Comment for an environment variable
};

/**
 * @brief The object type to which an attribute applies.
 *
 * Defines the different object types that can have attributes in a DBC file.
 */
enum class AttributeObjectType {
  UNDEFINED,       ///< Undefined or unknown attribute type
  NETWORK,         ///< Global attribute for the entire network
  NODE,            ///< Applies to a node (BU_)
  MESSAGE,         ///< Applies to a message (BO_)
  SIGNAL,          ///< Applies to a signal (SG_)
  ENV_VAR          ///< Applies to an environment variable (EV_)
};

/**
 * @brief Type of attribute value.
 *
 * Defines the different value types that an attribute can have.
 */
enum class AttributeValueType {
  INT,             ///< Integer value
  HEX,             ///< Hexadecimal value (treated as INT in many operations)
  FLOAT,           ///< Floating point value
  STRING,          ///< String value
  ENUM             ///< Enumeration value
};

/**
 * @brief Basic signal structure used across the parser.
 *
 * Represents a CAN signal with all its properties as defined in a DBC file.
 * This structure is used to store the parsed signal information and is included
 * in the Message structure.
 */
struct Signal {
  std::string name;                   ///< Signal name
  int start_bit = 0;                  ///< Start bit position
  int signal_size = 0;                ///< Size in bits (same as 'length')
  int length = 0;                     ///< Length in bits
  int byte_order = 1;                 ///< Byte order (1=little endian, 0=big endian)
  bool is_little_endian = true;       ///< Internal flag for byte order
  bool is_signed = false;             ///< Sign (+ or -)
  SignType sign = SignType::kUnsigned; ///< Sign type
  double factor = 1.0;                ///< Scaling factor
  double offset = 0.0;                ///< Offset
  double minimum = 0.0;               ///< Minimum value
  double maximum = 0.0;               ///< Maximum value
  std::string unit;                   ///< Unit (e.g., "km/h")
  std::vector<std::string> receivers; ///< Receiving nodes
  bool is_multiplexer = false;        ///< Whether signal is a multiplexer switch
  std::optional<int> multiplex_value; ///< Multiplexer value if signal is multiplexed
  MultiplexType multiplex_type = MultiplexType::kNone; ///< Multiplexing type
  int multiplex_value_int = -1;       ///< Integer representation of multiplex_value

  Signal() noexcept = default;
  ~Signal() noexcept = default;
};

/**
 * @brief Basic message structure.
 *
 * Represents a CAN message with all its properties as defined in a DBC file.
 * This structure contains the message metadata and a list of signals.
 */
struct Message {
  int id = 0;                 ///< Message ID
  std::string name;           ///< Message name
  int dlc = 0;                ///< Data Length Code (size in bytes)
  std::string sender;         ///< Sender node
  std::vector<Signal> signals;///< Signals in the message

  Message() noexcept = default;
  ~Message() noexcept = default;
};

/**
 * @brief Environment variable structure.
 *
 * Represents an environment variable (EV_) as defined in a DBC file.
 * Environment variables are used for various simulation and testing purposes.
 */
struct EnvironmentVariable {
  std::string name;                    ///< Environment variable name
  int var_type = 0;                    ///< Variable type
  double minimum = 0.0;                ///< Minimum value
  double maximum = 0.0;                ///< Maximum value
  std::string unit;                    ///< Unit
  double initial_value = 0.0;          ///< Initial value
  int ev_id = 0;                       ///< Environment variable ID
  std::string access_type;             ///< Access type
  std::string access_nodes;            ///< Access nodes as string

  EnvironmentVariable() noexcept = default;
  ~EnvironmentVariable() noexcept = default;
};

/**
 * @brief Environment variable data structure.
 *
 * Represents the data associated with an environment variable.
 */
struct EnvironmentVariableData {
  std::string name;                    ///< Environment variable name
  std::string data;                    ///< Environment variable data

  EnvironmentVariableData() noexcept = default;
  ~EnvironmentVariableData() noexcept = default;
};

/**
 * @brief Comment structure.
 *
 * Represents a comment (CM_) as defined in a DBC file. Comments can be associated
 * with different objects in the DBC file.
 */
struct Comment {
  CommentType type;                    ///< Comment type
  std::variant<
    std::monostate,                   ///< No identifier (NETWORK)
    std::string,                      ///< Object name (NODE, ENV_VAR)
    int,                              ///< Object ID (MESSAGE)
    std::pair<int, std::string>       ///< Message ID and signal name (SIGNAL)
  > identifier;                       ///< The object the comment is associated with
  std::string text;                    ///< Comment text

  Comment() noexcept = default;
  ~Comment() noexcept = default;
};

/**
 * @brief Node structure.
 *
 * Represents a node (ECU) as defined in a DBC file.
 */
struct Node {
  std::string name;                    ///< Node name

  Node() noexcept = default;
  ~Node() noexcept = default;
};

/**
 * @brief Bit timing structure.
 *
 * Represents the bit timing information (BS_) in a DBC file.
 */
struct BitTiming {
  int baudrate;                        ///< Baud rate in kbit/s
  double btr1_btr2;                    ///< Combined BTR1 and BTR2 register values

  BitTiming() noexcept = default;
  ~BitTiming() noexcept = default;
};

/**
 * @brief Enum to identify the type of object that a value description is associated with.
 */
enum class ValueDescriptionType {
  SIGNAL,        ///< Signal value description (VAL_ <message id> <signal name>)
  ENV_VAR        ///< Environment variable value description (VAL_ <env var name>)
};

/**
 * @brief Value table structure.
 *
 * Represents a value table (VAL_TABLE_) as defined in a DBC file.
 * Value tables define mappings from raw signal values to human-readable strings.
 */
struct ValueTable {
  std::string name;                    ///< Value table name
  std::map<int, std::string> values;   ///< Values mapping

  ValueTable() noexcept = default;
  ~ValueTable() noexcept = default;
};

/**
 * @brief Helper to convert common parser types to DBC file types.
 *
 * Utility class that provides type conversion functions between different
 * representations of the same DBC objects.
 */
class TypeConverter {
public:
  /**
   * @brief Default constructor is deleted to prevent instantiation.
   * 
   * This class only contains static utility methods and should not be instantiated.
   */
  TypeConverter() = delete;
  
  /**
   * @brief Destructor is deleted to prevent instantiation.
   */
  ~TypeConverter() = delete;

  // Prevent copying and moving
  TypeConverter(const TypeConverter&) = delete;
  TypeConverter& operator=(const TypeConverter&) = delete;
  TypeConverter(TypeConverter&&) = delete;
  TypeConverter& operator=(TypeConverter&&) = delete;

  // Add methods as needed for conversions between equivalent types
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_COMMON_TYPES_H_ 