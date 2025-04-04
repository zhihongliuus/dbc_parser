#ifndef SRC_DBC_PARSER_PARSER_NODES_PARSER_H_
#define SRC_DBC_PARSER_PARSER_NODES_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dbc_parser {
namespace parser {

// Represents a single node (ECU) in the DBC file
struct Node {
  std::string name;
};

// Contains all the nodes defined in the BU_ section
struct Nodes {
  std::vector<Node> nodes;
};

// Parser for the Nodes section (BU_) of a DBC file
class NodesParser {
 public:
  // Parses a nodes section from the given input
  // Returns std::nullopt if parsing fails
  static std::optional<Nodes> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_NODES_PARSER_H_ 