#ifndef DBC_PARSER_PARSER_VERSION_PARSER_H_
#define DBC_PARSER_PARSER_VERSION_PARSER_H_

#include <optional>
#include <string>
#include <string_view>

#include "dbc_parser/common/parser_base.h"
#include "dbc_parser/common/parser_state.h"

namespace dbc_parser {
namespace parser {

/**
 * @brief Structure to hold DBC file version information.
 *
 * Represents the VERSION section of a DBC file, which specifies
 * the version of the DBC file or the tool used to create it.
 */
struct Version {
  std::string version;  ///< Version string from the DBC file
};

/**
 * @brief Parser for the VERSION section in DBC files.
 *
 * Handles parsing of the VERSION section, which typically appears at the beginning
 * of a DBC file and specifies the version of the DBC file format or the tool
 * used to create the file.
 *
 * Example DBC version format:
 * VERSION "X.Y.Z"
 */
class VersionParser : public ParserBase {
 public:
  /**
   * @brief Parses a VERSION section from the given input string.
   * 
   * Takes a string containing a DBC VERSION section and parses it into a Version object.
   * The parser validates the syntax and extracts the version string.
   *
   * @param input String view containing the VERSION section to parse
   * @return std::optional<Version> A Version object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<Version> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_VERSION_PARSER_H_ 