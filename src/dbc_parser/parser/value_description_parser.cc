#include "src/dbc_parser/parser/value_description_parser.h"

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "tao/pegtl.hpp"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

namespace grammar {

// Basic grammar components
struct ws : pegtl::plus<pegtl::space> {};
struct optional_ws : pegtl::star<pegtl::space> {};
struct semicolon : pegtl::one<';'> {};

// Keywords
struct val_keyword : pegtl::string<'V', 'A', 'L', '_'> {};

// Identifiers and values
struct identifier : pegtl::seq<
  pegtl::alpha,
  pegtl::star<pegtl::sor<pegtl::alpha, pegtl::digit, pegtl::one<'_'>>>
> {};

struct message_id : pegtl::seq<pegtl::opt<pegtl::one<'-'>>, pegtl::plus<pegtl::digit>> {};
struct integer_value : pegtl::seq<pegtl::opt<pegtl::one<'-'>>, pegtl::plus<pegtl::digit>> {};

// String parsing for quoted strings
struct escaped_char : pegtl::seq<pegtl::one<'\\'>, pegtl::any> {};
struct regular_char : pegtl::not_one<'"', '\\'> {};
struct string_content : pegtl::star<pegtl::sor<escaped_char, regular_char>> {};
struct quoted_string : pegtl::seq<pegtl::one<'"'>, string_content, pegtl::one<'"'>> {};

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
};

// Action handlers
template<typename Rule>
struct action : pegtl::nothing<Rule> {};

// Message ID
template<>
struct action<grammar::message_id> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, value_description_state& state) {
    state.parsing_signal = true;
    state.result.type = ValueDescriptionType::SIGNAL;
    
    // Store message ID in state
    state.temp_message_id = std::stoi(in.string());
  }
};

// Identifier (for unquoted signal names or env var names)
template<>
struct action<grammar::identifier> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, value_description_state& state) {
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

// String content for quoted strings
template<>
struct action<grammar::string_content> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, value_description_state& state) {
    std::string content = in.string();
    
    if (state.in_value_pair) {
      // This is the description part of a value-description pair
      // Unescape any escaped characters
      std::string unescaped;
      for (size_t i = 0; i < content.length(); ++i) {
        if (content[i] == '\\' && i + 1 < content.length()) {
          // Skip backslash and add the escaped character
          unescaped += content[++i];
        } else {
          unescaped += content[i];
        }
      }
      
      // Add to the value descriptions map
      state.result.value_descriptions[state.current_value] = unescaped;
      state.in_value_pair = false;
    } else if (state.parsing_signal) {
      // This is a quoted signal name
      // Create a pair with the message ID and signal name
      state.result.identifier = std::make_pair(state.temp_message_id, content);
    }
  }
};

// Integer value
template<>
struct action<grammar::integer_value> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, value_description_state& state) {
    // Check if we've already processed message ID and signal name
    if (state.result.identifier.index() != std::variant_npos) {
      // We're in a value-description pair
      state.current_value = std::stoi(in.string());
      state.in_value_pair = true;
    }
  }
};

// Main parser implementation
std::optional<ValueDescription> ValueDescriptionParser::Parse(std::string_view input) {
  // Skip any leading whitespace
  auto it = input.begin();
  while (it != input.end() && std::isspace(*it)) {
    ++it;
  }
  
  // Empty input check
  if (it == input.end()) {
    return std::nullopt;
  }
  
  // Create PEGTL input
  pegtl::memory_input<> in(input.data(), input.size(), "");
  value_description_state state;
  
  // Parse the input
  try {
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