#ifndef DBC_PARSER_PARSER_VALUE_TABLE_PARSER_H_
#define DBC_PARSER_PARSER_VALUE_TABLE_PARSER_H_

#include <map>
#include <optional>
#include <string>
#include <string_view>

#include "dbc_parser/common/common_types.h"
#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

/**
 * @brief Parser for value tables (VAL_TABLE_) in DBC files.
 *
 * Handles parsing of value table entries, which define named lookup tables
 * mapping numeric values to human-readable string descriptions. Value tables
 * can be referenced by signals to avoid duplicating value descriptions.
 *
 * Example DBC value table format:
 * VAL_TABLE_ TableName 0 "Off" 1 "On" 2 "Error" 3 "Not Available";
 *
 * Value tables can then be referenced in signal definitions or type definitions
 * to reuse the same value mappings across multiple signals.
 */
class ValueTableParser : public ParserBase {
 public:
  /**
   * @brief Parses a value table from the given input string.
   *
   * Takes a string containing a DBC VAL_TABLE_ entry and parses it into a
   * ValueTable object. The parser validates the syntax and extracts the
   * table name and the mappings of values to descriptions.
   *
   * @param input String view containing the value table to parse
   * @return std::optional<ValueTable> A ValueTable object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<ValueTable> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_VALUE_TABLE_PARSER_H_ 