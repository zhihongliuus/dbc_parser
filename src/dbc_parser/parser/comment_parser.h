#ifndef SRC_DBC_PARSER_PARSER_COMMENT_PARSER_H_
#define SRC_DBC_PARSER_PARSER_COMMENT_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "src/dbc_parser/parser/common_types.h"

namespace dbc_parser {
namespace parser {

// Parser for comments (CM_, CMT_) in DBC files
class CommentParser {
 public:
  // Parses a comment section from the given input and returns a Comment object
  // Returns std::nullopt if parsing fails
  static std::optional<Comment> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_COMMENT_PARSER_H_ 