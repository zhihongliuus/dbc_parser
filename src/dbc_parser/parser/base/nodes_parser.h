#ifndef DBC_PARSER_PARSER_NODES_PARSER_H_
#define DBC_PARSER_PARSER_NODES_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "dbc_parser/common/common_types.h"
#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

// Parser for nodes list (BU_) in DBC files
class NodesParser : public ParserBase {
 public:
  // Parses a nodes section from the given input and returns a vector of Node objects
  // Returns std::nullopt if parsing fails
  [[nodiscard]] static std::optional<std::vector<Node>> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_NODES_PARSER_H_ 