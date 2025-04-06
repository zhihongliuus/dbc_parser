#ifndef DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_DATA_PARSER_H_
#define DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_DATA_PARSER_H_

#include <optional>
#include <string>
#include <string_view>

#include "dbc_parser/common/common_types.h"
#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

// Parser for the Environment Variable Data (ENVVAR_DATA_) section of a DBC file
class EnvironmentVariableDataParser : public ParserBase {
 public:
  // Parses an environment variable data section from the given input
  // Returns std::nullopt if parsing fails
  [[nodiscard]] static std::optional<EnvironmentVariableData> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_DATA_PARSER_H_