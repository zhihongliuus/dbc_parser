#ifndef DBC_PARSER_PARSER_SIGNAL_TYPE_DEF_PARSER_H_
#define DBC_PARSER_PARSER_SIGNAL_TYPE_DEF_PARSER_H_

#include <optional>
#include <string>
#include <string_view>

#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

/**
 * @brief Represents a signal type definition in the DBC file.
 *
 * Signal type definitions provide templates for commonly used signals with
 * predefined properties like size, byte order, scaling factors, etc.
 * They are defined in the SIGTYPE_ sections of a DBC file.
 */
struct SignalTypeDef {
  std::string name;          ///< Signal type name
  int size = 0;              ///< Signal size in bits
  int byte_order = 0;        ///< Signal byte order (1 for Intel/little endian, 0 for Motorola/big endian)
  std::string value_type;    ///< Value type (+ for unsigned, - for signed, etc.)
  double factor = 1.0;       ///< Factor for calculation of physical value
  double offset = 0.0;       ///< Offset for calculation of physical value
  double minimum = 0.0;      ///< Minimum physical value
  double maximum = 0.0;      ///< Maximum physical value
  std::string unit;          ///< Unit of physical value
  double default_value = 0.0; ///< Default value
  std::string value_table;   ///< Value table name (if any)
};

/**
 * @brief Parser for the Signal Type Definition (SIGTYPE_) section in DBC files.
 *
 * Handles parsing of the SIGTYPE_ section, which defines templates for signals
 * with common characteristics. These definitions can be referenced by signals
 * to avoid repeating the same properties multiple times.
 *
 * Example DBC signal type definition format:
 * SIGTYPE_ SignalTypeName : size byte_order value_type factor offset minimum maximum unit default_value value_table;
 * 
 * For example:
 * SIGTYPE_ Temperature : 16 0 - 0.1 0 -50 150 "Celsius" 20 TempValues;
 */
class SignalTypeDefParser : public ParserBase {
 public:
  /**
   * @brief Parses a signal type definition section from the given input string.
   *
   * Takes a string containing a DBC SIGTYPE_ section and parses it into a SignalTypeDef object.
   * The parser validates the syntax and extracts all the signal type properties.
   *
   * @param input String view containing the signal type definition section to parse
   * @return std::optional<SignalTypeDef> A SignalTypeDef object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<SignalTypeDef> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_SIGNAL_TYPE_DEF_PARSER_H_ 