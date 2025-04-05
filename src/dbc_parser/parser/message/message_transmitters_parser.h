#ifndef SRC_DBC_PARSER_PARSER_MESSAGE_TRANSMITTERS_PARSER_H_
#define SRC_DBC_PARSER_PARSER_MESSAGE_TRANSMITTERS_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "src/dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

// Represents a message transmitters definition in the DBC file
struct MessageTransmitters {
  int message_id = 0;
  std::vector<std::string> transmitters;
};

// Parser for the Message Transmitters (BO_TX_BU_) section of a DBC file
class MessageTransmittersParser : public ParserBase {
 public:
  // Parses a message transmitters section from the given input
  // Returns std::nullopt if parsing fails
  static std::optional<MessageTransmitters> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_MESSAGE_TRANSMITTERS_PARSER_H_ 