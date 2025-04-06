#ifndef DBC_PARSER_PARSER_MESSAGE_TRANSMITTERS_PARSER_H_
#define DBC_PARSER_PARSER_MESSAGE_TRANSMITTERS_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

/**
 * @brief Represents message transmitters definition from a DBC file.
 *
 * This structure stores information about which nodes (ECUs) can transmit 
 * a specific message, as defined in the BO_TX_BU_ section of a DBC file.
 */
struct MessageTransmitters {
  int message_id = 0;                  ///< ID of the message
  std::vector<std::string> transmitters; ///< List of node names that can transmit this message
};

/**
 * @brief Parser for the Message Transmitters (BO_TX_BU_) section in DBC files.
 *
 * Handles parsing of the BO_TX_BU_ section, which defines which nodes (ECUs)
 * are allowed to transmit specific messages on the CAN bus.
 *
 * Example DBC message transmitters format:
 * BO_TX_BU_ message_id : transmitter1[, transmitter2, ...];
 *
 * For example:
 * BO_TX_BU_ 123 : ECU1, ECU2;
 */
class MessageTransmittersParser : public ParserBase {
 public:
  /**
   * @brief Parses a message transmitters entry from the given input string.
   *
   * Takes a string containing a DBC BO_TX_BU_ section and parses it into a 
   * MessageTransmitters object. The parser validates the syntax and extracts
   * the message ID and list of transmitter nodes.
   *
   * @param input String view containing the message transmitters entry to parse
   * @return std::optional<MessageTransmitters> A MessageTransmitters object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<MessageTransmitters> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_MESSAGE_TRANSMITTERS_PARSER_H_ 