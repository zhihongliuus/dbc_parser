#include "src/dbc_parser/parser/signal_group_parser.h"

#include <string>
#include <string_view>
#include <optional>
#include <vector>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>

#include "src/dbc_parser/parser/common_grammar.h"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for parsing signal groups in DBC files.
namespace grammar {

// Use common grammar elements
using ws = common_grammar::ws;
using required_ws = common_grammar::req_ws;
using colon = common_grammar::colon;
using semicolon = common_grammar::semicolon;
using identifier = common_grammar::identifier;
using message_id = common_grammar::message_id;

// Keyword for signal groups.
struct sig_group_keyword : pegtl::string<'S', 'I', 'G', '_', 'G', 'R', 'O', 'U', 'P', '_'> {};

// Repetitions count.
struct repetitions : pegtl::plus<pegtl::digit> {};

// Signal list.
struct signal_name : pegtl::identifier {};
// Modified to handle non-empty signal lists
struct non_empty_signal_list : pegtl::list_must<signal_name, pegtl::one<','>, pegtl::space> {};

// The complete signal group rule with explicit check for required components
struct sig_group_rule : pegtl::seq<
    sig_group_keyword, ws, 
    message_id, required_ws, 
    identifier, required_ws, 
    repetitions, required_ws, 
    colon, ws,
    non_empty_signal_list, ws,
    semicolon, pegtl::eof
> {};

} // namespace grammar

// Class to accumulate the parsed data.
struct signal_group_state {
  std::optional<int> message_id;
  std::optional<std::string> group_name;
  std::optional<int> repetitions;
  std::vector<std::string> signals;

  bool is_complete() const {
    return message_id.has_value() && group_name.has_value() && 
           repetitions.has_value() && !signals.empty();
  }
};

// Actions to extract data during parsing.
template<typename Rule>
struct action : pegtl::nothing<Rule> {};

template<>
struct action<grammar::message_id> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_group_state& state) {
    if (!state.message_id.has_value()) {
      try {
        state.message_id = std::stoi(in.string());
      } catch (const std::exception& e) {
        // Error in conversion, leave as nullopt.
      }
    }
  }
};

template<>
struct action<grammar::identifier> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_group_state& state) {
    if (!state.group_name.has_value()) {
      state.group_name = in.string();
    }
  }
};

template<>
struct action<grammar::repetitions> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_group_state& state) {
    if (!state.repetitions.has_value()) {
      try {
        state.repetitions = std::stoi(in.string());
      } catch (const std::exception& e) {
        // Error in conversion, leave as nullopt.
      }
    }
  }
};

template<>
struct action<grammar::signal_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_group_state& state) {
    state.signals.push_back(in.string());
  }
};

std::optional<SignalGroup> SignalGroupParser::Parse(std::string_view input) {
  // Validate input using ParserBase method
  if (!ValidateInput(input)) {
    return std::nullopt;
  }

  // Additional validation specific to signal groups
  // Check for colon which is required in signal group definitions
  if (input.find(':') == std::string_view::npos) {
    return std::nullopt;
  }

  // Check for semicolon which is required at the end
  if (input.find(';') == std::string_view::npos) {
    return std::nullopt;
  }

  // Validate content between colon and semicolon (signals list can't be empty)
  size_t colon_pos = input.find(':');
  size_t semicolon_pos = input.find(';', colon_pos);
  if (semicolon_pos != std::string::npos && 
      semicolon_pos - colon_pos <= 2) {
    return std::nullopt;
  }

  signal_group_state state;
  
  // Create input for PEGTL parser using base class method
  pegtl::memory_input<> in = CreateInput(input, "SIG_GROUP_");
  
  try {
    pegtl::parse<grammar::sig_group_rule, action>(in, state);
  } catch (const pegtl::parse_error& e) {
    return std::nullopt;
  }

  if (!state.is_complete()) {
    return std::nullopt;
  }

  SignalGroup result;
  result.message_id = *state.message_id;
  result.group_name = *state.group_name;
  result.repetitions = *state.repetitions;
  result.signals = std::move(state.signals);

  return result;
}

} // namespace parser
} // namespace dbc_parser 