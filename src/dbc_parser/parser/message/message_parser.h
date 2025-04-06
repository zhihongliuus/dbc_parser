#ifndef DBC_PARSER_PARSER_MESSAGE_PARSER_H_
#define DBC_PARSER_PARSER_MESSAGE_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "dbc_parser/common/common_types.h"
#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

/**
 * @brief Parser for message (BO_) entries in DBC files.
 *
 * Handles parsing of message definitions in DBC files. A message represents
 * a CAN frame that can be transmitted on the bus, containing one or more signals.
 * The MessageParser extracts the message ID, name, DLC (Data Length Code),
 * transmitter, and associated signal definitions.
 *
 * Example DBC message format:
 * BO_ MessageID MessageName: DLC TransmitterName
 *  SG_ Signal1 : ...
 *  SG_ Signal2 : ...
 *
 * For example:
 * BO_ 100 EngineData: 8 ECU1
 *  SG_ EngineSpeed : 0|16@1+ (0.1,0) [0|6553.5] "rpm" ECU2
 *  SG_ EngineTemp : 16|8@1+ (0.5,-40) [-40|87.5] "degC" ECU2
 */
class MessageParser : public ParserBase {
 public:
  /**
   * @brief Parses a message section from the given input string.
   *
   * Takes a string containing a DBC BO_ section (including its signals) and parses
   * it into a Message object. The parser validates the message header syntax and
   * processes all signal definitions that follow the message header.
   *
   * @param input String view containing the message section to parse
   * @return std::optional<Message> A Message object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<Message> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_MESSAGE_PARSER_H_ 