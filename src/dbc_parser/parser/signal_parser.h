#ifndef DBC_PARSER_PARSER_SIGNAL_PARSER_H_
#define DBC_PARSER_PARSER_SIGNAL_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "src/dbc_parser/parser/common_types.h"

namespace dbc_parser {
namespace parser {

// Parser for SG_ entries in DBC files
class SignalParser {
 public:
  // Parses a signal string and returns a Signal object if parsing is successful
  // Returns std::nullopt if parsing fails
  //
  // Example format:
  // SG_ SignalName : 8|16@1+ (0.1,0) [0|655.35] "km/h" ECU1,ECU2
  static std::optional<Signal> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_SIGNAL_PARSER_H_ 