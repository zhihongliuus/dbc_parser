#include "dbc_parser/parser/base/nodes_parser.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/analyze.hpp"

#include "dbc_parser/common/common_grammar.h"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for BU_ (Nodes) parsing
namespace grammar {
// Use common grammar elements
using alpha = common_grammar::alpha;
using digit = common_grammar::digit;
using bu_keyword = common_grammar::bu_keyword;
using colon = common_grammar::colon;
using ws = common_grammar::ws;
using req_ws = common_grammar::req_ws;

// A node name character
struct node_name_char : pegtl::sor<
                         alpha,
                         digit,
                         pegtl::one<'_'>,
                         pegtl::one<'-'>
                       > {};

// A node name
struct node_name : pegtl::plus<node_name_char> {};

// Complete BU_ rule
struct bu_rule : pegtl::seq<
                   ws,
                   bu_keyword,
                   ws,
                   colon,
                   ws,
                   pegtl::opt<pegtl::list<node_name, req_ws>>,
                   ws,
                   pegtl::eof
                 > {};

} // namespace grammar

// Data structure to collect parsing results
struct nodes_state {
  std::vector<Node> nodes;
};

// PEGTL actions
template<typename Rule>
struct action : pegtl::nothing<Rule> {};

// Action for extracting node names
template<>
struct action<grammar::node_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, nodes_state& state) noexcept {
    Node node;
    node.name = in.string();
    state.nodes.push_back(node);
  }
};

std::optional<std::vector<Node>> NodesParser::Parse(std::string_view input) {
  // Validate input
  if (!ValidateInput(input)) {
    return std::nullopt;
  }

  // Create input for PEGTL parser
  pegtl::memory_input<> in = CreateInput(input, "BU_");
  
  // Create state to collect results
  nodes_state state;
  
  try {
    // Parse input using our grammar and actions
    if (pegtl::parse<grammar::bu_rule, action>(in, state)) {
      return state.nodes;
    }
  } catch (const pegtl::parse_error&) {
    // Parse error - return nullopt
    return std::nullopt;
  }
  
  return std::nullopt;
}

}  // namespace parser
}  // namespace dbc_parser 