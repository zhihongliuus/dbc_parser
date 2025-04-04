#ifndef SRC_DBC_PARSER_PARSER_BIT_TIMING_PARSER_H_
#define SRC_DBC_PARSER_PARSER_BIT_TIMING_PARSER_H_

#include <optional>
#include <string>
#include <string_view>

namespace dbc_parser {
namespace parser {

// Structure to hold bit timing information
struct BitTiming {
  int baudrate;     // Baud rate in kbit/s
  double btr1_btr2; // Combined BTR1 and BTR2 register values
};

// Parser for DBC BS_ (Bit Timing) section
class BitTimingParser {
 public:
  // Parses a BS_ string and returns a BitTiming object if successful
  static std::optional<BitTiming> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_BIT_TIMING_PARSER_H_ 