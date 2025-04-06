#ifndef DBC_PARSER_PARSER_NODES_PARSER_H_
#define DBC_PARSER_PARSER_NODES_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "dbc_parser/common/common_types.h"
#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

/**
 * @brief Parser for the nodes list (BU_) section in DBC files.
 *
 * Handles parsing of the BU_ (nodes) section, which defines the Electronic Control Units (ECUs)
 * present in the CAN network. Each node represents a device that can transmit or receive CAN messages.
 *
 * Example DBC nodes format:
 * BU_: ECU1 ECU2 GATEWAY SENSOR1
 *
 * These node names are referenced in other sections of the DBC file, such as message transmitters
 * and signal receivers.
 */
class NodesParser : public ParserBase {
 public:
  /**
   * @brief Parses a nodes section from the given input string.
   *
   * Takes a string containing a DBC BU_ section and parses it into a vector of Node objects.
   * The parser validates the syntax and extracts the node names.
   *
   * @param input String view containing the nodes section to parse
   * @return std::optional<std::vector<Node>> A vector of Node objects if parsing succeeds, 
   *         std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<std::vector<Node>> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_NODES_PARSER_H_ 