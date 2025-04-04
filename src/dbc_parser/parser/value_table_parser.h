#ifndef SRC_DBC_PARSER_PARSER_VALUE_TABLE_PARSER_H_
#define SRC_DBC_PARSER_PARSER_VALUE_TABLE_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace dbc_parser {
namespace parser {

// Represents a value table in the DBC file
struct ValueTable {
  std::string name;
  std::unordered_map<int, std::string> values;
};

// Parser for the Value Table (VAL_TABLE_) section of a DBC file
class ValueTableParser {
 public:
  // Parses a value table section from the given input
  // Returns std::nullopt if parsing fails
  static std::optional<ValueTable> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_VALUE_TABLE_PARSER_H_ 