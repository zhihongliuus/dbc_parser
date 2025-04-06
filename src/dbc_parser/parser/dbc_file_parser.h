#ifndef DBC_PARSER_PARSER_DBC_FILE_PARSER_H_
#define DBC_PARSER_PARSER_DBC_FILE_PARSER_H_

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "dbc_parser/common/common_types.h"
#include "dbc_parser/parser/message/message_parser.h"

namespace dbc_parser {
namespace parser {

// Forward declarations of supporting structures
struct Message;
struct ValueTable;
struct EnvironmentVariable;
struct Comment;
struct Attribute;
struct SignalGroup;

/**
 * @brief Structure to hold a complete parsed DBC file.
 *
 * Represents all the data contained in a DBC (CAN database) file. Each member variable 
 * corresponds to a different section of the DBC file format. This includes:
 * - Version information
 * - New symbols
 * - Bit timing
 * - Nodes/ECUs
 * - Messages and signals
 * - Value tables
 * - Environment variables
 * - Comments
 * - Attribute definitions and values
 * - Signal grouping
 */
struct DbcFile {
  /**
   * @brief Version string from the VERSION section.
   *
   * Format: VERSION "version_string"
   */
  std::string version;
  
  /**
   * @brief List of new symbols from the NS_ section.
   */
  std::vector<std::string> new_symbols;
  
  /**
   * @brief Internal structure for bit timing information.
   *
   * Represents the BS_ (bit timing) section of the DBC file.
   */
  struct BitTiming {
    int baudrate = 0;  ///< Baud rate in kbit/s
    int btr1 = 0;      ///< BTR1 register value
    int btr2 = 0;      ///< BTR2 register value

    BitTiming() noexcept = default;
    ~BitTiming() noexcept = default;
  };
  
  /**
   * @brief Optional bit timing information from the BS_ section.
   *
   * May be empty if the DBC file does not contain bit timing information.
   */
  std::optional<BitTiming> bit_timing;
  
  /**
   * @brief List of nodes/ECUs from the BU_ section.
   */
  std::vector<std::string> nodes;
  
  /**
   * @brief Value tables from the VAL_TABLE_ section.
   *
   * Maps value table names to value mappings (integer value to string description).
   */
  std::map<std::string, std::map<int, std::string>> value_tables;
  
  /**
   * @brief Internal structure for detailed message information.
   *
   * Represents a CAN message with its signals from the BO_ and SG_ sections.
   */
  struct MessageDef {
    int id = 0;                  ///< Message ID
    std::string name;            ///< Message name
    int size = 0;                ///< Message size in bytes
    std::string transmitter;     ///< Transmitting node
    std::vector<Signal> signals; ///< Signals contained in the message

    MessageDef() noexcept = default;
    ~MessageDef() noexcept = default;
  };
  
  /**
   * @brief Map of message IDs to detailed message definitions.
   *
   * Contains all BO_ message definitions with their SG_ signals.
   */
  std::map<int, MessageDef> messages_detailed;
  
  /**
   * @brief Map of message IDs to message names.
   *
   * Simplified representation of message data for faster lookups.
   */
  std::map<int, std::string> messages;
  
  /**
   * @brief Map of message IDs to transmitter lists.
   *
   * For tracking message transmitters (from BO_TX_BU_ sections).
   */
  std::map<int, std::vector<std::string>> message_transmitters;
  
  /**
   * @brief Internal structure for environment variables.
   *
   * Represents an environment variable from the EV_ section.
   */
  struct EnvVar {
    std::string name;                    ///< Environment variable name
    int type = 0;                        ///< Variable type
    double min_value = 0.0;              ///< Minimum value
    double max_value = 0.0;              ///< Maximum value
    std::string unit;                    ///< Unit
    double initial_value = 0.0;          ///< Initial value
    int ev_id = 0;                       ///< Environment variable ID
    std::string access_type;             ///< Access type
    std::vector<std::string> access_nodes; ///< Access nodes

    EnvVar() noexcept = default;
    ~EnvVar() noexcept = default;
  };
  
  /**
   * @brief Map of environment variable names to environment variable objects.
   *
   * Contains all EV_ environment variable definitions.
   */
  std::map<std::string, EnvVar> environment_variables;
  
  /**
   * @brief Internal structure for environment variable data.
   *
   * Represents data associated with an environment variable from the ENVVAR_DATA_ section.
   */
  struct EnvVarData {
    std::string data_name;  ///< Data name

    EnvVarData() noexcept = default;
    ~EnvVarData() noexcept = default;
  };
  
  /**
   * @brief Map of environment variable names to their data.
   *
   * Contains all ENVVAR_DATA_ definitions.
   */
  std::map<std::string, EnvVarData> environment_variable_data;
  
  /**
   * @brief Internal structure for comments.
   *
   * Represents a comment from the CM_ section.
   */
  struct CommentDef {
    CommentType type;             ///< Comment type (NETWORK, NODE, MESSAGE, SIGNAL, ENV_VAR)
    std::string object_name;      ///< Object name (for Node, EnvVar types)
    int object_id = 0;            ///< Object ID (for Message type)
    int signal_index = 0;         ///< Signal index (for Signal type within a Message)
    std::string text;             ///< Comment text

    CommentDef() noexcept = default;
    ~CommentDef() noexcept = default;
  };
  
  /**
   * @brief List of all comments in the DBC file.
   *
   * Contains all CM_ comment definitions.
   */
  std::vector<CommentDef> comments;
  
  /**
   * @brief Internal structure for attribute definitions.
   *
   * Represents an attribute definition from the BA_DEF_ section.
   */
  struct AttributeDef {
    std::string name;                   ///< Attribute name
    AttributeObjectType type;           ///< Object type this attribute applies to
    AttributeValueType value_type;      ///< Value type of this attribute
    std::vector<std::string> enum_values; ///< Enumeration values (for ENUM type)
    double min = 0.0;                   ///< Minimum value (for INT/FLOAT types)
    double max = 0.0;                   ///< Maximum value (for INT/FLOAT types)

    AttributeDef() noexcept = default;
    ~AttributeDef() noexcept = default;
  };
  
  /**
   * @brief List of all attribute definitions in the DBC file.
   *
   * Contains all BA_DEF_ attribute definitions.
   */
  std::vector<AttributeDef> attribute_definitions;
  
  /**
   * @brief Map of attribute names to their default values.
   *
   * Contains all BA_DEF_DEF_ attribute default definitions.
   */
  std::map<std::string, std::string> attribute_defaults;
  
  /**
   * @brief Internal structure for attribute values.
   *
   * Represents an attribute value from the BA_ section.
   */
  struct AttributeValue {
    std::string attr_name;              ///< Attribute name
    std::string node_name;              ///< Node name (for NODE attributes)
    int message_id = 0;                 ///< Message ID (for MESSAGE attributes)
    std::string signal_name;            ///< Signal name (for SIGNAL attributes)
    std::string env_var_name;           ///< Environment variable name (for ENV_VAR attributes)
    std::string value;                  ///< Attribute value

    AttributeValue() noexcept = default;
    ~AttributeValue() noexcept = default;
  };
  
  /**
   * @brief List of all attribute values in the DBC file.
   *
   * Contains all BA_ attribute value assignments.
   */
  std::vector<AttributeValue> attribute_values;
  
  /**
   * @brief Internal structure for value descriptions.
   *
   * Represents a value description from the VAL_ section.
   */
  struct ValueDescription {
    /**
     * @brief Type of value description (signal or environment variable).
     */
    ValueDescriptionType type = ValueDescriptionType::SIGNAL;
    
    /**
     * @brief Message ID and signal name for the value description.
     *
     * For SIGNAL type: message_id and signal_name are used
     * For ENV_VAR type: message_id will be -1 and signal_name contains the environment variable name
     */
    int message_id = 0;                 ///< Message ID (for SIGNAL type)
    std::string signal_name;            ///< Signal or env var name
    std::map<int, std::string> values;  ///< Map of raw values to descriptions

    ValueDescription() noexcept = default;
    ~ValueDescription() noexcept = default;
  };
  
  /**
   * @brief List of all value descriptions in the DBC file.
   *
   * Contains all VAL_ value description mappings.
   */
  std::vector<ValueDescription> value_descriptions;
  
  /**
   * @brief Internal structure for multiplexed signals.
   *
   * Represents a multiplexed signal definition from the SG_MUL_VAL_ section.
   */
  struct MultiplexedSignal {
    int message_id = 0;                         ///< Message ID
    std::string multiplexor_name;               ///< Multiplexor signal name
    std::string multiplexed_name;               ///< Multiplexed signal name
    std::vector<std::pair<int, int>> multiplexor_ranges; ///< Multiplexor value ranges

    MultiplexedSignal() noexcept = default;
    ~MultiplexedSignal() noexcept = default;
  };
  
  /**
   * @brief List of all multiplexed signal definitions in the DBC file.
   *
   * Contains all SG_MUL_VAL_ multiplexed signal definitions.
   */
  std::vector<MultiplexedSignal> multiplexed_signals;
  
  /**
   * @brief Internal structure for signal groups.
   *
   * Represents a signal group from the SIG_GROUP_ section.
   */
  struct SignalGroupDef {
    int message_id = 0;                   ///< Message ID
    std::string name;                     ///< Group name
    int repetitions = 1;                  ///< Number of repetitions
    std::vector<std::string> signal_names; ///< Names of signals in the group

    SignalGroupDef() noexcept = default;
    ~SignalGroupDef() noexcept = default;
  };
  
  /**
   * @brief List of all signal group definitions in the DBC file.
   *
   * Contains all SIG_GROUP_ signal group definitions.
   */
  std::vector<SignalGroupDef> signal_groups;
  
  /**
   * @brief Internal structure for signal value types.
   *
   * Represents a signal value type from the SIG_VALTYPE_ section.
   */
  struct SignalValueType {
    int message_id = 0;         ///< Message ID
    std::string signal_name;    ///< Signal name
    int value_type = 0;         ///< Value type (0: Signed/Unsigned, 1: IEEE Float, 2: IEEE Double)

    SignalValueType() noexcept = default;
    ~SignalValueType() noexcept = default;
  };
  
  /**
   * @brief List of all signal value type definitions in the DBC file.
   *
   * Contains all SIG_VALTYPE_ signal value type definitions.
   */
  std::vector<SignalValueType> signal_value_types;

  /**
   * @brief Default constructor.
   */
  DbcFile() noexcept = default;
  
  /**
   * @brief Default destructor.
   */
  ~DbcFile() noexcept = default;
};

/**
 * @brief Main parser class for DBC files.
 *
 * This class is responsible for parsing an entire DBC file and constructing
 * a DbcFile object that represents all the data contained in the file.
 * It orchestrates the parsing of different sections using specialized parsers.
 */
class DbcFileParser {
 public:
  /**
   * @brief Default constructor.
   */
  DbcFileParser() noexcept = default;
  
  /**
   * @brief Default destructor.
   */
  ~DbcFileParser() noexcept = default;

  // Disable copy and move operations
  DbcFileParser(const DbcFileParser&) = delete;
  DbcFileParser& operator=(const DbcFileParser&) = delete;
  DbcFileParser(DbcFileParser&&) = delete;
  DbcFileParser& operator=(DbcFileParser&&) = delete;

  /**
   * @brief Parse DBC file content into a structured representation.
   *
   * Takes a string containing a complete DBC file and parses it into a DbcFile structure.
   * The parser validates the syntax and extracts all the information from the different
   * sections of the DBC file.
   *
   * @param input String view containing the DBC file content to parse
   * @return std::optional<DbcFile> A DbcFile object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] std::optional<DbcFile> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_DBC_FILE_PARSER_H_ 