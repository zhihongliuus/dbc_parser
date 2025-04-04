#ifndef SRC_DBC_PARSER_PARSER_MESSAGE_PARSER_H_
#define SRC_DBC_PARSER_PARSER_MESSAGE_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "src/dbc_parser/parser/signal_parser.h"

namespace dbc_parser {
namespace parser {

// Represents a message in the DBC file
struct Message {
  int id = 0;
  std::string name;
  int dlc = 0;  // Data Length Code (number of bytes)
  std::string sender;
  std::vector<Signal> signals;
};

// Parser for the Message (BO_) section of a DBC file
class MessageParser {
 public:
  // Parses a message section from the given input
  // Returns std::nullopt if parsing fails
  static std::optional<Message> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_MESSAGE_PARSER_H_ 