#ifndef DBC_PARSER_PARSER_SIGNAL_PARSER_H_
#define DBC_PARSER_PARSER_SIGNAL_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "dbc_parser/common/common_types.h"
#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

/**
 * @brief Parser for signal (SG_) entries in DBC files.
 *
 * Handles parsing of signal definitions within message blocks in DBC files.
 * Signals define the individual data fields within a CAN message, including
 * their bit position, length, scaling, and other properties.
 *
 * Example DBC signal format:
 * SG_ SignalName : StartBit|Length@ByteOrder+ (Factor,Offset) [Min|Max] "Unit" ReceiverList
 * 
 * Components of a signal definition:
 * - SignalName: Name of the signal
 * - StartBit: Bit position where the signal starts
 * - Length: Length of the signal in bits
 * - ByteOrder: 1 for Intel/little endian, 0 for Motorola/big endian
 * - Sign: + for unsigned, - for signed
 * - Factor, Offset: For conversion between raw and physical values (Physical = Raw * Factor + Offset)
 * - Min, Max: Minimum and maximum physical values
 * - Unit: Unit of the physical value
 * - ReceiverList: Comma-separated list of nodes that receive this signal
 *
 * For example:
 * SG_ EngineSpeed : 8|16@1+ (0.1,0) [0|655.35] "rpm" ECU1,ECU2
 */
class SignalParser : public ParserBase {
 public:
  /**
   * @brief Parses a signal definition from the given input string.
   *
   * Takes a string containing a DBC SG_ definition and parses it into a Signal object.
   * The parser validates the syntax and extracts all signal properties such as
   * name, bit position, length, byte order, scaling factors, value range,
   * unit, and receiver list.
   *
   * @param input String view containing the signal definition to parse
   * @return std::optional<Signal> A Signal object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<Signal> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_SIGNAL_PARSER_H_ 