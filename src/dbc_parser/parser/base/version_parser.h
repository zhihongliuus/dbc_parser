#ifndef SRC_DBC_PARSER_PARSER_VERSION_PARSER_H_
#define SRC_DBC_PARSER_PARSER_VERSION_PARSER_H_

#include <optional>
#include <string>
#include <string_view>

#include "src/dbc_parser/common/parser_base.h"
#include "src/dbc_parser/common/parser_state.h"

namespace dbc_parser {
namespace parser {

// Structure to hold Version information
struct Version {
  std::string version;
};

// Parser for DBC VERSION
class VersionParser : public ParserBase {
 public:
  /**
   * Parses a VERSION string and returns a Version object if successful
   * 
   * @param input The input string to parse
   * @return An optional containing the Version object if successful, nullopt otherwise
   */
  [[nodiscard]] static std::optional<Version> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_VERSION_PARSER_H_ 