#ifndef SRC_DBC_PARSER_PARSER_VERSION_PARSER_H_
#define SRC_DBC_PARSER_PARSER_VERSION_PARSER_H_

#include <optional>
#include <string>
#include <string_view>

namespace dbc_parser {
namespace parser {

// Structure to hold Version information
struct Version {
  std::string version;
};

// Parser for DBC VERSION
class VersionParser {
 public:
  // Parses a VERSION string and returns a Version object if successful
  static std::optional<Version> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_VERSION_PARSER_H_ 