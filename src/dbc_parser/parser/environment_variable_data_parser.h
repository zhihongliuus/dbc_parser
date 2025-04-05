#ifndef SRC_DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_DATA_PARSER_H_
#define SRC_DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_DATA_PARSER_H_

#include <optional>
#include <string>
#include <string_view>

#include "src/dbc_parser/parser/common_types.h"
#include "src/dbc_parser/parser/parser_base.h"

namespace dbc_parser {
namespace parser {

// Parser for the Environment Variable Data (ENVVAR_DATA_) section of a DBC file
class EnvironmentVariableDataParser : public ParserBase {
 public:
  // Parses an environment variable data section from the given input
  // Returns std::nullopt if parsing fails
  static std::optional<EnvironmentVariableData> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_DATA_PARSER_H_