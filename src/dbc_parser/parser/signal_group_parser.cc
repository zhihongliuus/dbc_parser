#include "src/dbc_parser/parser/signal_group_parser.h"

#include <string>
#include <string_view>
#include <optional>
#include <vector>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for parsing signal groups in DBC files.
namespace grammar {

// Whitespace handling.
struct ws : pegtl::star<pegtl::space> {};
struct required_ws : pegtl::plus<pegtl::space> {};

// Keyword for signal groups.
struct sig_group_keyword : pegtl::string<'S', 'I', 'G', '_', 'G', 'R', 'O', 'U', 'P', '_'> {};

// Message ID, which can be signed.
struct sign : pegtl::opt<pegtl::one<'-'>> {};
struct digits : pegtl::plus<pegtl::digit> {};
struct message_id : pegtl::seq<sign, digits> {};

// Identifier (for group name and signal names).
struct identifier : pegtl::identifier {};

// Repetitions count.
struct repetitions : pegtl::plus<pegtl::digit> {};

// Signal list.
struct signal_name : pegtl::identifier {};
// Ensure we have at least one signal name
struct non_empty_signal_names : pegtl::list_must<signal_name, pegtl::one<','>, pegtl::space> {};

// Colon and semicolon.
struct colon : pegtl::one<':'> {};
struct semicolon : pegtl::one<';'> {};

// The complete signal group rule.
struct sig_group_rule : pegtl::seq<
    sig_group_keyword, ws, 
    message_id, required_ws, 
    identifier, required_ws, 
    repetitions, required_ws, 
    colon, ws,
    non_empty_signal_names, ws,
    semicolon, pegtl::eof> {};

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
  signal_group_state state;
  
  // Special handling for missing semicolon
  if (input.length() > 0 && input[input.length() - 1] != ';') {
    return std::nullopt;
  }
  
  // Special handling for missing colon
  size_t colon_pos = input.find(':');
  if (colon_pos == std::string_view::npos) {
    return std::nullopt;
  }
  
  // Special handling for empty signal list
  if (input.find(':', colon_pos) != std::string_view::npos &&
      input.find(';', colon_pos) != std::string_view::npos &&
      input.find(';', colon_pos) - input.find(':', colon_pos) <= 2) {
    return std::nullopt;
  }

  pegtl::memory_input in(input.data(), input.size(), "");
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