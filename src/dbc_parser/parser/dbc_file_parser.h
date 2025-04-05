#ifndef SRC_DBC_PARSER_PARSER_DBC_FILE_PARSER_H_
#define SRC_DBC_PARSER_PARSER_DBC_FILE_PARSER_H_

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "src/dbc_parser/common/common_types.h"
#include "src/dbc_parser/parser/message/message_parser.h"

namespace dbc_parser {
namespace parser {

// Forward declarations of supporting structures
struct Message;
struct ValueTable;
struct EnvironmentVariable;
struct Comment;
struct Attribute;
struct SignalGroup;

// Structure to hold a complete DBC file data
struct DbcFile {
  // VERSION "version_string"
  std::string version;
  
  // NS_ : list of new symbols
  std::vector<std::string> new_symbols;
  
  // BS_ : bit timing information
  struct BitTiming {
    int baudrate = 0;
    int btr1 = 0;
    int btr2 = 0;

    BitTiming() noexcept = default;
    ~BitTiming() noexcept = default;
  };
  std::optional<BitTiming> bit_timing;
  
  // BU_: nodes/ECUs
  std::vector<std::string> nodes;
  
  // VAL_TABLE_ : value tables
  std::map<std::string, std::map<int, std::string>> value_tables;
  
  // BO_ : messages and SG_: signals within messages
  struct MessageDef {
    int id = 0;
    std::string name;
    int size = 0;
    std::string transmitter;
    std::vector<Signal> signals;

    MessageDef() noexcept = default;
    ~MessageDef() noexcept = default;
  };
  std::map<int, MessageDef> messages_detailed;
  

  // Maps for storing other DBC elements
  // These are simplified representations for the parser
  std::map<int, std::string> messages;
  std::map<int, std::vector<std::string>> message_transmitters;
  
  // EV_ : environment variables
  struct EnvVar {
    std::string name;
    int type = 0;
    double min_value = 0.0;
    double max_value = 0.0;
    std::string unit;
    double initial_value = 0.0;
    int ev_id = 0;
    std::string access_type;
    std::vector<std::string> access_nodes;

    EnvVar() noexcept = default;
    ~EnvVar() noexcept = default;
  };
  std::map<std::string, EnvVar> environment_variables;
  
  // ENVVAR_DATA_ : environment variable data
  struct EnvVarData {
    std::string data_name;

    EnvVarData() = default;
    ~EnvVarData() = default;
  };
  std::map<std::string, EnvVarData> environment_variable_data;
  
  // CM_ : comments
  struct CommentDef {
    CommentType type;
    std::string object_name; // For Node, EnvVar
    int object_id = 0;       // For Message
    int signal_index = 0;    // For Signal (within Message)
    std::string text;

    CommentDef() noexcept = default;
    ~CommentDef() noexcept = default;
  };
  std::vector<CommentDef> comments;
  
  // BA_DEF_ : attribute definitions
  struct AttributeDef {
    std::string name;
    AttributeObjectType type;
    AttributeValueType value_type;
    std::vector<std::string> enum_values; // For Enum type
    double min = 0.0;                    // For Int/Float
    double max = 0.0;                    // For Int/Float

    AttributeDef() noexcept = default;
    ~AttributeDef() noexcept = default;
  };
  std::vector<AttributeDef> attribute_definitions;
  
  // BA_DEF_DEF_ : attribute defaults
  std::map<std::string, std::string> attribute_defaults;
  
  // BA_ : attribute values
  struct AttributeValue {
    std::string attr_name;
    std::string node_name;    // For Node-specific
    int message_id = 0;       // For Message-specific
    std::string signal_name;  // For Signal-specific
    std::string env_var_name; // For EnvVar-specific
    std::string value;

    AttributeValue() noexcept = default;
    ~AttributeValue() noexcept = default;
  };
  std::vector<AttributeValue> attribute_values;
  
  // VAL_ : signal values
  struct ValueDescription {
    // Type of value description
    ValueDescriptionType type = ValueDescriptionType::SIGNAL;
    
    // For SIGNAL type: message_id and signal_name are used
    // For ENV_VAR type: message_id will be -1 and signal_name contains the environment variable name
    int message_id = 0;
    std::string signal_name;
    std::map<int, std::string> values;

    ValueDescription() noexcept = default;
    ~ValueDescription() noexcept = default;
  };
  std::vector<ValueDescription> value_descriptions;
  
  // SG_MUL_VAL_ : multiplexed signals
  struct MultiplexedSignal {
    int message_id = 0;
    std::string multiplexor_name;
    std::string multiplexed_name;
    std::vector<std::pair<int, int>> multiplexor_ranges;

    MultiplexedSignal() noexcept = default;
    ~MultiplexedSignal() noexcept = default;
  };
  std::vector<MultiplexedSignal> multiplexed_signals;
  
  // SIG_GROUP_ : signal groups
  struct SignalGroupDef {
    int message_id = 0;
    std::string name;
    int repetitions = 1;
    std::vector<std::string> signal_names;

    SignalGroupDef() noexcept = default;
    ~SignalGroupDef() noexcept = default;
  };
  std::vector<SignalGroupDef> signal_groups;
  
  // SIG_VALTYPE_ : signal value types
  struct SignalValueType {
    int message_id = 0;
    std::string signal_name;
    int value_type = 0; // 0: Signed/Unsigned, 1: IEEE Float, 2: IEEE Double

    SignalValueType() noexcept = default;
    ~SignalValueType() noexcept = default;
  };
  std::vector<SignalValueType> signal_value_types;

  // Main struct constructor/destructor
  DbcFile() = default;
  ~DbcFile() = default;
};

// Main parser class for DBC files
class DbcFileParser {
 public:
  DbcFileParser() = default;
  ~DbcFileParser() = default;

  // Disable copy operations
  DbcFileParser(const DbcFileParser&) = delete;
  DbcFileParser& operator=(const DbcFileParser&) = delete;

  // Parse a DBC file content and return a DbcFile structure
  // Returns nullopt if parsing fails
  [[nodiscard]] std::optional<DbcFile> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_DBC_FILE_PARSER_H_ 