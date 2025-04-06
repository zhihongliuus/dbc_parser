#ifndef DBC_PARSER_PARSER_VALUE_DESCRIPTION_PARSER_H_
#define DBC_PARSER_PARSER_VALUE_DESCRIPTION_PARSER_H_

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "dbc_parser/common/common_types.h"
#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

/**
 * @brief Represents value descriptions for signals or environment variables in a DBC file.
 *
 * Value descriptions map numeric values to human-readable string descriptions,
 * providing meaningful interpretations for specific signal or environment variable values.
 * These are defined in the VAL_ sections of DBC files.
 */
struct ValueDescription {
  ValueDescriptionType type;   ///< Type of value description (SIGNAL or ENV_VAR)
  
  /**
   * @brief Identifier for the signal or environment variable
   *
   * This variant holds different types of identifiers depending on the value description type:
   * - For SIGNAL type: std::pair<int, std::string> containing (message_id, signal_name)
   * - For ENV_VAR type: std::string containing the environment variable name
   */
  std::variant<
    std::pair<int, std::string>,   ///< SIGNAL: (message_id, signal_name)
    std::string                    ///< ENV_VAR: env_var_name
  > identifier;
  
  std::map<int, std::string> value_descriptions;  ///< Map of integer values to their textual descriptions
};

/**
 * @brief Parser for value descriptions (VAL_) in DBC files.
 *
 * Handles parsing of value description entries, which map numeric values to
 * human-readable string descriptions for signals or environment variables.
 * These descriptions help interpret raw values in a more meaningful way.
 *
 * Example DBC value description formats:
 * - VAL_ 123 "SignalName" 0 "Off" 1 "On" 2 "Error";     (Signal value description)
 * - VAL_ "EnvVarName" 0 "Inactive" 1 "Active";          (Environment variable value description)
 */
class ValueDescriptionParser : public ParserBase {
 public:
  /**
   * @brief Parses a value description from the given input string.
   *
   * Takes a string containing a DBC VAL_ entry and parses it into a
   * ValueDescription object. The parser validates the syntax and extracts
   * the type, identifier, and mappings of values to descriptions.
   *
   * @param input String view containing the value description to parse
   * @return std::optional<ValueDescription> A ValueDescription object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<ValueDescription> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_VALUE_DESCRIPTION_PARSER_H_ 