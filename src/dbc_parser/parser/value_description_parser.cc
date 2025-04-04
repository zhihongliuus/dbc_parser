#include "src/dbc_parser/parser/value_description_parser.h"

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/analyze.hpp"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for Value Description parsing (VAL_)
namespace grammar {

// Basic whitespace rule
struct ws : pegtl::star<pegtl::space> {};

// VAL_ keyword
struct val_keyword : pegtl::string<'V', 'A', 'L', '_'> {};

// Message ID
struct message_id : pegtl::seq<
                      pegtl::opt<pegtl::one<'-'>>,
                      pegtl::plus<pegtl::digit>
                    > {};

// Integer (for values)
struct integer_value : pegtl::seq<
                         pegtl::opt<pegtl::one<'-'>>,
                         pegtl::plus<pegtl::digit>
                       > {};

// Valid identifier character
struct id_char : pegtl::sor<
                   pegtl::alpha,
                   pegtl::digit,
                   pegtl::one<'_'>,
                   pegtl::one<'-'>
                 > {};

// Signal name
struct signal_name : pegtl::plus<id_char> {};

// Environment variable name
struct env_var_name : pegtl::plus<id_char> {};

// Rules for quoted strings with escaping
struct escaped_char : pegtl::seq<pegtl::one<'\\'>, pegtl::any> {};
struct regular_char : pegtl::not_one<'"', '\\'> {};
struct string_content : pegtl::star<pegtl::sor<escaped_char, regular_char>> {};
struct quoted_string : pegtl::seq<pegtl::one<'"'>, string_content, pegtl::one<'"'>> {};

// Value-description pair
struct value_desc_pair : pegtl::seq<integer_value, ws, quoted_string> {};

// Multiple value-description pairs
struct value_desc_pairs : pegtl::list<value_desc_pair, ws> {};

// Semicolon at the end
struct semicolon : pegtl::one<';'> {};

// Signal value description rule: VAL_ <message_id> <signal_name> <value_desc_pairs>;
struct signal_val_rule : pegtl::seq<
                          val_keyword,
                          ws,
                          message_id,            // Message ID
                          ws,
                          signal_name,           // Signal name
                          ws,
                          value_desc_pairs,      // Value-description pairs
                          ws,
                          semicolon
                        > {};

// Environment variable value description rule: VAL_ <env_var_name> <value_desc_pairs>;
struct env_var_val_rule : pegtl::seq<
                            val_keyword,
                            ws,
                            env_var_name,         // Environment variable name
                            ws,
                            value_desc_pairs,     // Value-description pairs
                            ws,
                            semicolon
                          > {};

// Complete VAL_ rule - try signal first, then env var
struct val_rule : pegtl::sor<signal_val_rule, env_var_val_rule> {};

} // namespace grammar

// Data structure to collect parsing results
struct value_description_state {
  ValueDescription value_description;
  bool in_signal_rule = false;
  int message_id = 0;
  std::string signal_or_env_name;
  int current_value = 0;
  bool parsing_value = true;  // Flag to know we're parsing a value (not a message ID)
  bool in_value_pair = false; // Flag to know we're inside a value-description pair
};

// PEGTL actions
template<typename Rule>
struct action : pegtl::nothing<Rule> {};

// Signal rule was matched
template<>
struct action<grammar::signal_val_rule> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, value_description_state& state) {
    state.in_signal_rule = true;
    state.value_description.type = ValueDescriptionType::SIGNAL;
  }
};

// Env var rule was matched
template<>
struct action<grammar::env_var_val_rule> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, value_description_state& state) {
    state.in_signal_rule = false;
    state.value_description.type = ValueDescriptionType::ENV_VAR;
  }
};

// Extract message ID
template<>
struct action<grammar::message_id> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, value_description_state& state) {
    state.message_id = std::stoi(in.string());
  }
};

// Extract value in value-description pair
template<>
struct action<grammar::integer_value> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, value_description_state& state) {
    state.current_value = std::stoi(in.string());
    state.in_value_pair = true;
  }
};

// Extract signal name
template<>
struct action<grammar::signal_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, value_description_state& state) {
    state.signal_or_env_name = in.string();
    // Construct signal identifier
    state.value_description.identifier = std::make_pair(state.message_id, state.signal_or_env_name);
  }
};

// Extract environment variable name
template<>
struct action<grammar::env_var_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, value_description_state& state) {
    state.signal_or_env_name = in.string();
    // Set environment variable identifier
    state.value_description.identifier = state.signal_or_env_name;
  }
};

// Extract description string
template<>
struct action<grammar::string_content> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, value_description_state& state) {
    if (state.in_value_pair) {
      // This is a description in a value-description pair
      std::string description = in.string();
      
      // Unescape any escaped characters
      std::string unescaped;
      for (size_t i = 0; i < description.length(); ++i) {
        if (description[i] == '\\' && i + 1 < description.length()) {
          // Skip backslash and add the escaped character
          unescaped += description[++i];
        } else {
          unescaped += description[i];
        }
      }
      
      // Add to value descriptions map
      state.value_description.value_descriptions[state.current_value] = unescaped;
      state.in_value_pair = false;
    }
  }
};

// Track the start of a value-description pair
template<>
struct action<grammar::value_desc_pair> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, value_description_state& state) {
    state.in_value_pair = false; // Reset for next pair
  }
};

std::optional<ValueDescription> ValueDescriptionParser::Parse(std::string_view input) {
  if (input.empty()) {
    return std::nullopt;
  }

  // Create input for PEGTL parser
  pegtl::memory_input<> in(input.data(), input.size(), "VAL_");
  
  // Create state to collect results
  value_description_state state;
  state.in_value_pair = false;
  
  try {
    // Parse input using our grammar and actions
    if (pegtl::parse<grammar::val_rule, action>(in, state)) {
      // Ensure we have at least one value description
      if (state.value_description.value_descriptions.empty()) {
        return std::nullopt;
      }
      return state.value_description;
    }
  } catch (const pegtl::parse_error&) {
    // Parse error - return nullopt
    return std::nullopt;
  }
  
  return std::nullopt;
}

}  // namespace parser
}  // namespace dbc_parser 