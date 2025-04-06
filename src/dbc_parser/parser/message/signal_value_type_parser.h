#ifndef DBC_PARSER_PARSER_SIGNAL_VALUE_TYPE_PARSER_H_
#define DBC_PARSER_PARSER_SIGNAL_VALUE_TYPE_PARSER_H_

#include <optional>
#include <string>
#include <string_view>

#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

/**
 * @brief Represents a signal value type entry from the SIG_VALTYPE_ section in a DBC file.
 *
 * The SignalValueType defines the extended type information for signals, 
 * particularly for floating-point signals. It associates a message ID and 
 * signal name with a specific signal type (integer, float, or double).
 */
struct SignalValueType {
  int message_id = 0;         ///< ID of the message containing the signal
  std::string signal_name;    ///< Name of the signal
  int type = 0;               ///< Signal type (0: integer, 1: float, 2: double)
};

/**
 * @brief Parser for the Signal Value Type (SIG_VALTYPE_) section in DBC files.
 *
 * Handles parsing of the SIG_VALTYPE_ section, which defines extended type
 * information for signals, particularly for floating-point values.
 *
 * Example DBC signal value type format:
 * SIG_VALTYPE_ message_id signal_name : type;
 *
 * For example:
 * SIG_VALTYPE_ 123 EngineTemperature : 1; // Float type
 */
class SignalValueTypeParser : public ParserBase {
 public:
  /**
   * @brief Parses a signal value type entry from the given input string.
   *
   * Takes a string containing a DBC SIG_VALTYPE_ section and parses it into a 
   * SignalValueType object. The parser validates the syntax and extracts the
   * message ID, signal name, and type information.
   *
   * @param input String view containing the signal value type entry to parse
   * @return std::optional<SignalValueType> A SignalValueType object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<SignalValueType> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_SIGNAL_VALUE_TYPE_PARSER_H_ 