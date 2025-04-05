#ifndef SRC_DBC_PARSER_PARSER_VALUE_TABLE_PARSER_H_
#define SRC_DBC_PARSER_PARSER_VALUE_TABLE_PARSER_H_

#include <map>
#include <optional>
#include <string>
#include <string_view>

#include "src/dbc_parser/parser/common_types.h"
#include "src/dbc_parser/parser/parser_base.h"

namespace dbc_parser {
namespace parser {

// Parser for value table (VAL_TABLE_) in DBC files
class ValueTableParser : public ParserBase {
 public:
  // Parses a value table section from the given input and returns a ValueTable object
  // Returns std::nullopt if parsing fails
  static std::optional<ValueTable> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_VALUE_TABLE_PARSER_H_ 