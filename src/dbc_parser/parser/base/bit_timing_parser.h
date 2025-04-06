#ifndef DBC_PARSER_PARSER_BIT_TIMING_PARSER_H_
#define DBC_PARSER_PARSER_BIT_TIMING_PARSER_H_

#include <optional>
#include <string>
#include <string_view>

#include "dbc_parser/common/common_types.h"
#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

/**
 * @brief Parser for bit timing (BS_) section in DBC files.
 *
 * Handles parsing of the BS_ (bit timing) section, which specifies the CAN bus
 * timing parameters. This section is typically used to define the baud rate and
 * other timing parameters for the CAN network.
 *
 * Example DBC bit timing format:
 * BS_: 500 0 0
 * 
 * Where the values represent:
 * - Baud rate (in kbit/s)
 * - BTR1 register value
 * - BTR2 register value
 */
class BitTimingParser : public ParserBase {
 public:
  /**
   * @brief Parses a bit timing section from the given input string.
   *
   * Takes a string containing a DBC BS_ section and parses it into a BitTiming object.
   * The parser validates the syntax and extracts the baud rate and BTR values.
   *
   * @param input String view containing the bit timing section to parse
   * @return std::optional<BitTiming> A BitTiming object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<BitTiming> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_BIT_TIMING_PARSER_H_ 