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

// Parser for comments (CM_, CMT_) in DBC files
class CommentParser : public ParserBase {
 public:
  // Parses a comment section from the given input and returns a Comment object
  // Returns std::nullopt if parsing fails
  [[nodiscard]] static std::optional<Comment> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_COMMENT_PARSER_H_ 