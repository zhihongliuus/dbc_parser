#ifndef DBC_PARSER_PARSER_SIGNAL_GROUP_PARSER_H_
#define DBC_PARSER_PARSER_SIGNAL_GROUP_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

/**
 * @brief Represents a signal group definition from a DBC file.
 *
 * Signal groups allow for the logical grouping of related signals within a message.
 * These are defined in the SIG_GROUP_ sections of DBC files and are useful for 
 * organizing signals that share a common purpose or function.
 */
struct SignalGroup {
  int message_id;                  ///< The message ID associated with this signal group
  std::string group_name;          ///< The name of the signal group
  int repetitions;                 ///< The repetition count (typically 1 for non-multiplexed groups)
  std::vector<std::string> signals; ///< The list of signal names in this group
};

/**
 * @brief Parser for signal groups (SIG_GROUP_) in DBC files.
 *
 * Handles parsing of signal group definitions, which organize related signals
 * within a message into logical groups. Signal groups are useful for documentation
 * and for handling related signals together in applications.
 *
 * Example DBC signal group format:
 * SIG_GROUP_ message_id group_name repetitions : signal1 signal2 ... ;
 *
 * For example:
 * SIG_GROUP_ 100 EngineGroup 1 : EngineSpeed EngineTemp EnginePressure;
 */
class SignalGroupParser : public ParserBase {
 public:
  /**
   * @brief Parses a signal group definition from the given input string.
   *
   * Takes a string containing a DBC SIG_GROUP_ section and parses it into a 
   * SignalGroup object. The parser validates the syntax and extracts the
   * message ID, group name, repetition count, and list of signals in the group.
   *
   * @param input String view containing the signal group definition to parse
   * @return std::optional<SignalGroup> A SignalGroup object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<SignalGroup> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_SIGNAL_GROUP_PARSER_H_ 