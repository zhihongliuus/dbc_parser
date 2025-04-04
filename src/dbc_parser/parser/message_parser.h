#ifndef SRC_DBC_PARSER_PARSER_MESSAGE_PARSER_H_
#define SRC_DBC_PARSER_PARSER_MESSAGE_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "src/dbc_parser/parser/common_types.h"

namespace dbc_parser {
namespace parser {

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