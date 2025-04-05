#ifndef SRC_DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_PARSER_H_
#define SRC_DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_PARSER_H_

#include <optional>
#include <string>
#include <string_view>

#include "src/dbc_parser/common/common_types.h"
#include "src/dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

// Parser for the Environment Variable (EV_) section of a DBC file
class EnvironmentVariableParser : public ParserBase {
 public:
  // Parses an environment variable section from the given input
  // Returns std::nullopt if parsing fails
  static std::optional<EnvironmentVariable> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_PARSER_H_ 