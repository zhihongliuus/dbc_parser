#ifndef DBC_PARSER_PARSER_COMMENT_PARSER_H_
#define DBC_PARSER_PARSER_COMMENT_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "dbc_parser/common/common_types.h"
#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

/**
 * @brief Parser for comments in DBC files.
 *
 * Handles parsing of CM_ (comment) entries in DBC files. These comments can be associated with
 * different objects: networks, nodes, messages, signals, or environment variables.
 * 
 * Example DBC comment formats:
 * - Network comment: CM_ "This is a network comment";
 * - Node comment: CM_ BU_ NodeName "This is a node comment";
 * - Message comment: CM_ BO_ 123 "This is a message comment";
 * - Signal comment: CM_ SG_ 123 SignalName "This is a signal comment";
 * - Environment variable comment: CM_ EV_ EnvVarName "This is an environment variable comment";
 */
class CommentParser : public ParserBase {
 public:
  /**
   * @brief Parses a comment section from the given input string.
   *
   * Takes a string containing a DBC comment section and parses it into a Comment object.
   * The parser validates the syntax and extracts the comment type, identifier, and text.
   *
   * @param input String view containing the comment section to parse
   * @return std::optional<Comment> A Comment object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<Comment> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_COMMENT_PARSER_H_ 