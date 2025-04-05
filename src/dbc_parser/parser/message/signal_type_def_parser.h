#ifndef SRC_DBC_PARSER_PARSER_SIGNAL_TYPE_DEF_PARSER_H_
#define SRC_DBC_PARSER_PARSER_SIGNAL_TYPE_DEF_PARSER_H_

#include <optional>
#include <string>
#include <string_view>

#include "src/dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

// Represents a signal type definition in the DBC file
struct SignalTypeDef {
  // Signal type name
  std::string name;
  
  // Signal size in bits
  int size = 0;
  
  // Signal byte order (1 for big endian (Intel), 0 for little endian (Motorola))
  int byte_order = 0;
  
  // Value type (+ for unsigned, - for signed, etc.)
  std::string value_type;
  
  // Factor for calculation of physical value
  double factor = 1.0;
  
  // Offset for calculation of physical value
  double offset = 0.0;
  
  // Minimum physical value
  double minimum = 0.0;
  
  // Maximum physical value
  double maximum = 0.0;
  
  // Unit of physical value
  std::string unit;
  
  // Default value
  double default_value = 0.0;
  
  // Value table name (if any)
  std::string value_table;
};

// Parser for the Signal Type Definition (SIG_TYPE_DEF_) section of a DBC file
class SignalTypeDefParser : public ParserBase {
 public:
  // Parses a signal type definition section from the given input
  // Returns std::nullopt if parsing fails
  static std::optional<SignalTypeDef> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_SIGNAL_TYPE_DEF_PARSER_H_ 