#ifndef DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_PARSER_H_
#define DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_PARSER_H_

#include <optional>
#include <string>
#include <string_view>

#include "dbc_parser/common/common_types.h"
#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

/**
 * @brief Parser for environment variable (EV_) entries in DBC files.
 *
 * Handles parsing of environment variable definitions, which represent 
 * variables that are shared across the network but are not part of CAN messages.
 * Environment variables can be used for system configuration, diagnostics,
 * or other network-wide parameters.
 *
 * Example DBC environment variable format:
 * EV_ EnvVarName: EnvVarType [Min|Max] Unit InitValue ID AccessType AccessNodes;
 *
 * Components:
 * - EnvVarName: Name of the environment variable
 * - EnvVarType: Data type (0: integer, 1: float, 2: string)
 * - Min, Max: Value range
 * - Unit: Unit of the variable
 * - InitValue: Initial value
 * - ID: Unique identifier
 * - AccessType: Access permissions (DUMMY_NODE_VECTOR0, etc.)
 * - AccessNodes: List of nodes that can access the variable
 *
 * For example:
 * EV_ ProtocolVersion: 0 [0|255] "" 1 DUMMY_NODE_VECTOR0 Vector__XXX;
 */
class EnvironmentVariableParser : public ParserBase {
 public:
  /**
   * @brief Parses an environment variable definition from the given input string.
   *
   * Takes a string containing a DBC EV_ definition and parses it into an 
   * EnvironmentVariable object. The parser validates the syntax and extracts
   * all environment variable properties.
   *
   * @param input String view containing the environment variable definition to parse
   * @return std::optional<EnvironmentVariable> An EnvironmentVariable object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<EnvironmentVariable> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_PARSER_H_ 