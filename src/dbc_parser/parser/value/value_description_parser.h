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

// Structure to hold value description data from DBC file
struct ValueDescription {
  ValueDescriptionType type;   // Type of value description
  
  // Variant to hold the relevant identifiers based on value description type
  // - For SIGNAL, both message id and signal name are stored
  // - For ENV_VAR, the environment variable name is stored
  std::variant<
    std::pair<int, std::string>,   // SIGNAL: (message_id, signal_name)
    std::string                    // ENV_VAR: env_var_name
  > identifier;
  
  // Map of integer values to their textual descriptions
  std::map<int, std::string> value_descriptions;
};

// Parser for VAL_ entries in DBC files
class ValueDescriptionParser : public ParserBase {
 public:
  // Parses a value description string and returns a ValueDescription object if parsing is successful
  // Returns std::nullopt if parsing fails
  //
  // Example formats:
  // VAL_ 123 SignalName 0 "Off" 1 "On" 2 "Error";
  // VAL_ EnvVarName 0 "Inactive" 1 "Active";
  [[nodiscard]] static std::optional<ValueDescription> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_VALUE_DESCRIPTION_PARSER_H_ 