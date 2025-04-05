#ifndef DBC_PARSER_PARSER_SIGNAL_GROUP_PARSER_H_
#define DBC_PARSER_PARSER_SIGNAL_GROUP_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "src/dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

// Represents a signal group in a DBC file.
struct SignalGroup {
  // The message ID associated with this signal group.
  int message_id;
  // The name of the signal group.
  std::string group_name;
  // The repetition count.
  int repetitions;
  // The list of signal names in this group.
  std::vector<std::string> signals;
};

// Parser for signal groups (SIG_GROUP_) in DBC files.
class SignalGroupParser : public ParserBase {
 public:
  // Parses a signal group section from a DBC file.
  // Returns the parsed signal group if successful, or std::nullopt if parsing fails.
  static std::optional<SignalGroup> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_SIGNAL_GROUP_PARSER_H_ 