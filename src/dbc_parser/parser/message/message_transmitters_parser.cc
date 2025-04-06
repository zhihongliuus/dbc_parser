#include "dbc_parser/parser/message/message_transmitters_parser.h"

#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/analyze.hpp"

#include "dbc_parser/common/common_grammar.h"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for BO_TX_BU_ (Message Transmitters) parsing
namespace grammar {

// Use common grammar elements
using ws = common_grammar::ws;
using integer = common_grammar::integer;
using colon = common_grammar::colon;
using semicolon = common_grammar::semicolon;

// BO_TX_BU_ keyword
struct bo_tx_bu_keyword : pegtl::string<'B', 'O', '_', 'T', 'X', '_', 'B', 'U', '_'> {};

// Valid identifier character
struct id_char : pegtl::sor<
                   pegtl::alpha,
                   pegtl::digit,
                   pegtl::one<'_'>,
                   pegtl::one<'-'>
                 > {};

// Node name
struct node_name : pegtl::plus<id_char> {};

// Transmitter list (comma-separated)
struct transmitter_list : pegtl::list<
                            node_name,
                            pegtl::one<','>,
                            pegtl::space
                          > {};

// Complete BO_TX_BU_ rule
struct bo_tx_bu_rule : pegtl::seq<
                         ws,
                         bo_tx_bu_keyword,
                         ws,
                         integer,            // Message ID
                         ws,
                         colon,
                         ws,
                         pegtl::opt<transmitter_list>,
                         ws,
                         pegtl::opt<semicolon>,
                         ws,
                         pegtl::eof
                       > {};

} // namespace grammar

// Data structure to collect parsing results
struct transmitters_state {
  MessageTransmitters transmitters;
};

// PEGTL actions
template<typename Rule>
struct action : pegtl::nothing<Rule> {};

// Message ID
template<>
struct action<grammar::integer> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, transmitters_state& state) {
    state.transmitters.message_id = std::stoi(in.string());
  }
};

// Node name (transmitter)
template<>
struct action<grammar::node_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, transmitters_state& state) {
    state.transmitters.transmitters.push_back(in.string());
  }
};

std::optional<MessageTransmitters> MessageTransmittersParser::Parse(std::string_view input) {
  // Validate input using base class method
  if (!ValidateInput(input)) {
    return std::nullopt;
  }

  // Create input for PEGTL parser using base class method
  pegtl::memory_input<> in = CreateInput(input, "BO_TX_BU_");
  
  // Create state to collect results
  transmitters_state state;
  
  try {
    // Parse input using our grammar and actions
    if (pegtl::parse<grammar::bo_tx_bu_rule, action>(in, state)) {
      return state.transmitters;
    }
  } catch (const pegtl::parse_error&) {
    // Parse error - return nullopt
    return std::nullopt;
  }
  
  return std::nullopt;
}

}  // namespace parser
}  // namespace dbc_parser 