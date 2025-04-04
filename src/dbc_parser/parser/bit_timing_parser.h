#ifndef SRC_DBC_PARSER_PARSER_BIT_TIMING_PARSER_H_
#define SRC_DBC_PARSER_PARSER_BIT_TIMING_PARSER_H_

#include <optional>
#include <string>
#include <string_view>

#include "src/dbc_parser/parser/common_types.h"

namespace dbc_parser {
namespace parser {

// Parser for bit timing (BS_) in DBC files
class BitTimingParser {
 public:
  // Parses a bit timing section from the given input and returns a BitTiming object
  // Returns std::nullopt if parsing fails
  static std::optional<BitTiming> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_BIT_TIMING_PARSER_H_ 