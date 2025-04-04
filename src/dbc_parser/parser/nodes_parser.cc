#include "src/dbc_parser/parser/nodes_parser.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/analyze.hpp"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for BU_ (Nodes) parsing
namespace grammar {

// BU_ keyword
struct bu_keyword : pegtl::string<'B', 'U', '_'> {};

// Colon separator
struct colon : pegtl::one<':'> {};

// A node name character
struct node_name_char : pegtl::sor<
                         pegtl::alpha,
                         pegtl::digit,
                         pegtl::one<'_'>,
                         pegtl::one<'-'>
                       > {};

// A node name
struct node_name : pegtl::plus<node_name_char> {};

// Complete BU_ rule
struct bu_rule : pegtl::seq<
                   pegtl::opt<pegtl::star<pegtl::space>>,
                   bu_keyword,
                   pegtl::opt<pegtl::star<pegtl::space>>,
                   colon,
                   pegtl::opt<pegtl::star<pegtl::space>>,
                   pegtl::opt<pegtl::list<node_name, pegtl::plus<pegtl::space>>>,
                   pegtl::opt<pegtl::star<pegtl::space>>,
                   pegtl::eof
                 > {};

} // namespace grammar

// Data structure to collect parsing results
struct nodes_state {
  Nodes nodes;
};

// PEGTL actions
template<typename Rule>
struct action : pegtl::nothing<Rule> {};

// Action for extracting node names
template<>
struct action<grammar::node_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, nodes_state& state) {
    Node node;
    node.name = in.string();
    state.nodes.nodes.push_back(node);
  }
};

std::optional<Nodes> NodesParser::Parse(std::string_view input) {
  if (input.empty()) {
    return std::nullopt;
  }

  // Create input for PEGTL parser
  pegtl::memory_input<> in(input.data(), input.size(), "BU_");
  
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