#ifndef SRC_DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_PARSER_H_
#define SRC_DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_PARSER_H_

#include <optional>
#include <string>
#include <string_view>

namespace dbc_parser {
namespace parser {

// Represents an environment variable in the DBC file
struct EnvironmentVariable {
  std::string name;
  int var_type = 0;      // Integer type (0 = Integer, 1 = Float, 2 = String)
  double minimum = 0.0;
  double maximum = 0.0;
  std::string unit;
  int initial_value = 0;
  int ev_id = 0;         // Environment variable ID
  std::string access_type;  // Access type (e.g., DUMMY_NODE_VECTOR0)
  std::string access_nodes; // Comma-separated list of nodes with access
};

// Parser for the Environment Variable (EV_) section of a DBC file
class EnvironmentVariableParser {
 public:
  // Parses an environment variable section from the given input
  // Returns std::nullopt if parsing fails
  static std::optional<EnvironmentVariable> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_ENVIRONMENT_VARIABLE_PARSER_H_ 