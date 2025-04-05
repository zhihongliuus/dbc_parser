#include "src/dbc_parser/parser/comment_parser.h"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for parsing comments in DBC files
namespace grammar {

// Whitespace handling
struct ws : pegtl::star<pegtl::space> {};
struct required_ws : pegtl::plus<pegtl::space> {};

// Keyword for comments
struct cm_prefix : pegtl::string<'C', 'M', '_'> {};

// Object type indicators
struct bu_type : pegtl::string<'B', 'U', '_'> {};
struct bo_type : pegtl::string<'B', 'O', '_'> {};
struct sg_type : pegtl::string<'S', 'G', '_'> {};
struct ev_type : pegtl::string<'E', 'V', '_'> {};

// Identifiers
struct node_name : pegtl::identifier {};
struct env_var_name : pegtl::identifier {};
struct signal_name : pegtl::identifier {};

// Message ID - can be signed
struct sign : pegtl::opt<pegtl::one<'-'>> {};
struct digits : pegtl::plus<pegtl::digit> {};
struct message_id : pegtl::seq<sign, digits> {};

// String with quotes and escape sequences
struct escaped_char : pegtl::seq<pegtl::one<'\\'>, pegtl::any> {};
struct unescaped_char : pegtl::not_one<'"', '\\'> {};
struct string_content : pegtl::star<pegtl::sor<escaped_char, unescaped_char>> {};
struct quoted_string : pegtl::seq<
    pegtl::one<'"'>,
    string_content,
    pegtl::one<'"'>
> {};

// Semicolon
struct semicolon : pegtl::one<';'> {};

// Different comment types
struct network_comment : pegtl::seq<
    cm_prefix, ws,
    quoted_string, ws,
    semicolon
> {};

struct node_comment : pegtl::seq<
    cm_prefix, ws,
    bu_type, required_ws,
    quoted_string, ws,  // Use quoted_string for node name
    quoted_string, ws,
    semicolon
> {};

struct message_comment : pegtl::seq<
    cm_prefix, ws,
    bo_type, ws,  // More flexible whitespace handling
    message_id, ws,  // Don't require whitespace here
    quoted_string, ws,
    semicolon
> {};

struct signal_comment : pegtl::seq<
    cm_prefix, ws,
    sg_type, required_ws,
    message_id, required_ws,
    quoted_string, ws,  // Use quoted_string for signal name
    quoted_string, ws,
    semicolon
> {};

struct env_var_comment : pegtl::seq<
    cm_prefix, ws,
    ev_type, required_ws,
    quoted_string, ws,  // Use quoted_string for env var name
    quoted_string, ws,
    semicolon
> {};

// The complete comment rule
struct comment_rule : pegtl::sor<
    signal_comment,
    message_comment,
    node_comment,
    env_var_comment,
    network_comment
> {};

} // namespace grammar

// Class to accumulate parsed data
struct comment_state {
  CommentType type = CommentType::NETWORK;
  std::variant<
    std::monostate,                // NETWORK
    std::string,                   // NODE, ENV_VAR
    int,                           // MESSAGE
    std::pair<int, std::string>    // SIGNAL
  > identifier;
  std::optional<std::string> text;
  
  // Additional tracking for quoted strings
  bool is_first_quoted_string = true;

  bool is_complete() const {
    return text.has_value() && 
        ((type == CommentType::NETWORK && std::holds_alternative<std::monostate>(identifier)) ||
        ((type == CommentType::NODE || type == CommentType::ENV_VAR) && std::holds_alternative<std::string>(identifier)) ||
        (type == CommentType::MESSAGE && std::holds_alternative<int>(identifier)) ||
        (type == CommentType::SIGNAL && std::holds_alternative<std::pair<int, std::string>>(identifier)));
  }
};

// Debug helper to trace parsing
template<typename Rule>
struct tracer : pegtl::normal<Rule> {
  template<typename Input>
  static void start(const Input& in, comment_state& state) {
    std::cout << "Starting to match rule " << typeid(Rule).name() << " at position " << in.position() << std::endl;
    pegtl::normal<Rule>::start(in, state);
  }

  template<typename Input>
  static void success(const Input& in, comment_state& state) {
    std::cout << "Successfully matched rule " << typeid(Rule).name() << " at position " << in.position() << std::endl;
    pegtl::normal<Rule>::success(in, state);
  }

  template<typename Input>
  static void failure(const Input& in, comment_state& state) {
    std::cout << "Failed to match rule " << typeid(Rule).name() << " at position " << in.position() << std::endl;
    pegtl::normal<Rule>::failure(in, state);
  }
};

// Actions to extract data during parsing
template<typename Rule>
struct action : pegtl::nothing<Rule> {};

template<>
struct action<grammar::quoted_string> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, comment_state& state) {
    std::string content = in.string();
    if (content.size() >= 2) {
      // Remove surrounding quotes
      content = content.substr(1, content.size() - 2);
      
      // Unescape special characters
      std::string unescaped;
      bool escaped = false;
      for (char c : content) {
        if (escaped) {
          unescaped += c;
          escaped = false;
        } else if (c == '\\') {
          escaped = true;
        } else {
          unescaped += c;
        }
      }
      
      // For NODE, ENV_VAR, and SIGNAL types, we need to handle identifier differently
      if (state.is_first_quoted_string) {
        if (state.type == CommentType::NETWORK) {
          // For NETWORK, the first string is always the text
          state.text = unescaped;
          state.is_first_quoted_string = false;
        } else if (state.type == CommentType::NODE) {
          // For NODE, the first string is the node name
          state.identifier = unescaped;
          state.is_first_quoted_string = false;
        } else if (state.type == CommentType::ENV_VAR) {
          // For ENV_VAR, the first string is the env var name
          state.identifier = unescaped;
          state.is_first_quoted_string = false;
        } else if (state.type == CommentType::MESSAGE) {
          // For MESSAGE, the first string is the comment text since message_id is already set
          state.text = unescaped;
          state.is_first_quoted_string = false;
        } else if (state.type == CommentType::SIGNAL && 
                  std::holds_alternative<std::pair<int, std::string>>(state.identifier)) {
          // For SIGNAL, update the signal name in the pair
          auto& pair = std::get<std::pair<int, std::string>>(state.identifier);
          pair.second = unescaped;
          state.is_first_quoted_string = false;
        }
      } else {
        // For all types, the second string is the comment text
        state.text = unescaped;
      }
    }
  }
};

template<>
struct action<grammar::bu_type> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, comment_state& state) {
    state.type = CommentType::NODE;
    state.identifier = std::monostate{};  // Reset the variant
    state.is_first_quoted_string = true;  // Reset string counter
  }
};

template<>
struct action<grammar::bo_type> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, comment_state& state) {
    state.type = CommentType::MESSAGE;
    state.identifier = std::monostate{};  // Reset the variant
    state.is_first_quoted_string = true;  // Reset string counter
  }
};

template<>
struct action<grammar::sg_type> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, comment_state& state) {
    state.type = CommentType::SIGNAL;
    state.identifier = std::monostate{};  // Reset the variant
    state.is_first_quoted_string = true;  // Reset string counter
  }
};

template<>
struct action<grammar::ev_type> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, comment_state& state) {
    state.type = CommentType::ENV_VAR;
    state.identifier = std::monostate{};  // Reset the variant
    state.is_first_quoted_string = true;  // Reset string counter
  }
};

template<>
struct action<grammar::message_id> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, comment_state& state) {
    if (state.type == CommentType::MESSAGE || state.type == CommentType::SIGNAL) {
      try {
        int id = std::stoi(in.string());
        if (state.type == CommentType::MESSAGE) {
          state.identifier = id;
        } else if (state.type == CommentType::SIGNAL) {
          // For signal comments, the identifier is a pair of message ID and signal name
          // Initialize with the message ID and add the signal name later
          state.identifier = std::make_pair(id, "");
        }
      } catch (const std::exception& e) {
        // Error in conversion, leave as default
      }
    }
  }
};

std::optional<Comment> CommentParser::Parse(std::string_view input) {
  // Quick check for required syntax elements
  if (input.empty() || input[input.length() - 1] != ';') {
    return std::nullopt;
  }

  comment_state state;
  
  // Debug the input
  std::cout << "Parsing comment: '" << input << "'" << std::endl;
  
  pegtl::memory_input in(input.data(), input.size(), "");
  try {
    // Use tracer for detailed debugging
    pegtl::parse<grammar::comment_rule, action, tracer>(in, state);
    // pegtl::parse<grammar::comment_rule, action>(in, state);
  } catch (const pegtl::parse_error& e) {
    std::cerr << "Parse error: " << e.what() << std::endl;
    return std::nullopt;
  }

  if (!state.is_complete() || (state.text.has_value() && state.text->empty())) {
    std::cerr << "Incomplete comment parsing" << std::endl;
    return std::nullopt;
  }

  Comment result;
  result.type = state.type;
  result.identifier = state.identifier;
  result.text = *state.text;

  return result;
}

} // namespace parser
} // namespace dbc_parser 