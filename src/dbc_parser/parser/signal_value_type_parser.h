#ifndef SRC_DBC_PARSER_PARSER_SIGNAL_VALUE_TYPE_PARSER_H_
#define SRC_DBC_PARSER_PARSER_SIGNAL_VALUE_TYPE_PARSER_H_

#include <optional>
#include <string>
#include <string_view>

#include "src/dbc_parser/parser/parser_base.h"

namespace dbc_parser {
namespace parser {

// Represents a signal value type in the DBC file (SIG_VALTYPE_)
struct SignalValueType {
  // Message ID
  int message_id = 0;
  
  // Signal name
  std::string signal_name;
  
  // Signal type (0: integer, 1: float, 2: double)
  int type = 0;
};

// Parser for the Signal Value Type (SIG_VALTYPE_) section of a DBC file
class SignalValueTypeParser : public ParserBase {
 public:
  // Parses a signal value type section from the given input
  // Returns std::nullopt if parsing fails
  static std::optional<SignalValueType> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_SIGNAL_VALUE_TYPE_PARSER_H_ 