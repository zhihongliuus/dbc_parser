#ifndef DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_DATA_PARSER_H_
#define DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_DATA_PARSER_H_

#include <optional>
#include <string>
#include <string_view>

#include "dbc_parser/common/common_types.h"
#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

/**
 * @brief Parser for environment variable data (ENVVAR_DATA_) entries in DBC files.
 *
 * Handles parsing of environment variable data definitions, which provide additional
 * information about environment variables such as data type, unit, and other
 * metadata. These entries extend the basic environment variable definitions.
 *
 * Example DBC environment variable data format:
 * ENVVAR_DATA_ EnvVarName: DataName
 *
 * Where:
 * - EnvVarName: Name of the environment variable this data refers to
 * - DataName: Additional data associated with the environment variable
 *
 * For example:
 * ENVVAR_DATA_ ProtocolVersion: 0;
 */
class EnvironmentVariableDataParser : public ParserBase {
 public:
  /**
   * @brief Parses environment variable data from the given input string.
   *
   * Takes a string containing a DBC ENVVAR_DATA_ definition and parses it into
   * an EnvironmentVariableData object. The parser validates the syntax and
   * extracts the environment variable name and associated data.
   *
   * @param input String view containing the environment variable data to parse
   * @return std::optional<EnvironmentVariableData> An EnvironmentVariableData object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<EnvironmentVariableData> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_DATA_PARSER_H_