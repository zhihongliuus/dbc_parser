#include "src/dbc_parser/parser/value/value_description_parser.h"

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "tao/pegtl.hpp"
#include "src/dbc_parser/common/common_grammar.h"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

namespace grammar {

// Use common grammar elements
using ws = common_grammar::ws;
using optional_ws = common_grammar::opt_ws;
using semicolon = common_grammar::semicolon;
using message_id = common_grammar::message_id;
using identifier = common_grammar::identifier;
using quoted_string = common_grammar::quoted_string;

// Keywords
struct val_keyword : pegtl::string<'V', 'A', 'L', '_'> {};

// Values
struct integer_value : pegtl::seq<pegtl::opt<pegtl::one<'-'>>, pegtl::plus<pegtl::digit>> {};

// Value-description pair: <integer_value> "<description>"
struct value_desc_pair : pegtl::seq<
  integer_value,
  ws,
  quoted_string
> {};

// Signal value description rule: VAL_ <message_id> <signal_name> <value_desc_pairs>;
struct signal_value_desc : pegtl::seq<
  val_keyword,
  ws,
  message_id,
  ws,
  pegtl::sor<
    quoted_string,      // Signal name can be quoted
    identifier          // or unquoted identifier
  >,
  pegtl::star<
    pegtl::seq<
      ws,
      value_desc_pair
    >
  >,
  optional_ws,
  semicolon
> {};

// Environment variable value description rule: VAL_ <env_var_name> <value_desc_pairs>;
struct env_var_value_desc : pegtl::seq<
  val_keyword,
  ws,
  identifier,
  pegtl::star<
    pegtl::seq<
      ws,
      value_desc_pair
    >
  >,
  optional_ws,
  semicolon
> {};

// Main VAL_ rule (try signal first, then env var)
struct val_rule : pegtl::sor<signal_value_desc, env_var_value_desc> {};

} // namespace grammar

// State for parsing
struct value_description_state {
  ValueDescription result;
  int current_value = 0;
  bool in_value_pair = false;
  bool parsing_signal = false;
  int temp_message_id = 0;  // Store temp message ID in state instead of result
  std::string current_description; // To store the current description being parsed
};

// Action handlers
template<typename Rule>
struct action : pegtl::nothing<Rule> {};

// Message ID
template<>
struct action<grammar::message_id> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, value_description_state& state) noexcept {
    state.parsing_signal = true;
    state.result.type = ValueDescriptionType::SIGNAL;
    
    // Store message ID in state
    try {
      state.temp_message_id = std::stoi(in.string());
    } catch (const std::exception&) {
      // Default to 0 if conversion fails
      state.temp_message_id = 0;
    }
  }
};

// Identifier (for unquoted signal names or env var names)
template<>
struct action<grammar::identifier> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, value_description_state& state) noexcept {
    std::string name = in.string();
    
    if (state.parsing_signal) {
      // For signals, create a pair with the message ID and signal name
      state.result.identifier = std::make_pair(state.temp_message_id, name);
    } else {
      // For environment variables, just store the name
      state.result.type = ValueDescriptionType::ENV_VAR;
      state.result.identifier = name;
    }
  }
};

// Handle quoted string - extract the full string with quotes
template<>
struct action<grammar::quoted_string> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, value_description_state& state) noexcept {
    if (state.in_value_pair) {
      // This is a quoted string in a value-description pair
      std::string quoted = in.string();
      // Use ParserBase::UnescapeString to properly handle escaped quotes
      state.result.value_descriptions[state.current_value] = ParserBase::UnescapeString(quoted);
      state.in_value_pair = false;
    } else if (state.parsing_signal) {
      // This is a quoted signal name
      // Extract the content without quotes and store it
      std::string quoted = in.string();
      std::string name = ParserBase::UnescapeString(quoted);
      state.result.identifier = std::make_pair(state.temp_message_id, name);
    }
  }
};

// Integer value
template<>
struct action<grammar::integer_value> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, value_description_state& state) noexcept {
    // Check if we've already processed message ID and signal name
    if (state.result.identifier.index() != std::variant_npos) {
      // We're in a value-description pair
      try {
        state.current_value = std::stoi(in.string());
      } catch (const std::exception&) {
        // Default to 0 if conversion fails
        state.current_value = 0;
      }
      state.in_value_pair = true;
    }
  }
};

// Main parser implementation
std::optional<ValueDescription> ValueDescriptionParser::Parse(std::string_view input) {
  // Validate input using ParserBase method
  if (!ValidateInput(input)) {
    return std::nullopt;
  }
  
  // Create state for parsing
  value_description_state state;
  
  try {
    // Create input for PEGTL parser using base class method
    pegtl::memory_input<> in = CreateInput(input, "VAL_");
    
    // Parse the input
    const bool success = pegtl::parse<grammar::val_rule, action>(in, state);
    
    if (success && !state.result.value_descriptions.empty()) {
      // Parsing succeeded and we have at least one value description
      return state.result;
    }
  } catch (const pegtl::parse_error& e) {
    // Parse error - return nullopt
    return std::nullopt;
  }
  
  return std::nullopt;
}

}  // namespace parser
}  // namespace dbc_parser 