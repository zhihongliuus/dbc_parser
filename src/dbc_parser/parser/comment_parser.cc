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
    quoted_string, ws,
    quoted_string, ws,
    semicolon
> {};

struct message_comment : pegtl::seq<
    cm_prefix, ws,
    bo_type, ws,
    message_id, ws,
    quoted_string, ws,
    semicolon
> {};

struct signal_comment : pegtl::seq<
    cm_prefix, ws,
    sg_type, required_ws,
    message_id, required_ws,
    quoted_string, ws,
    quoted_string, ws,
    semicolon
> {};

struct env_var_comment : pegtl::seq<
    cm_prefix, ws,
    ev_type, required_ws,
    quoted_string, ws,
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
struct CommentState {
  CommentType type = CommentType::NETWORK;
  std::variant<
    std::monostate,                // NETWORK
    std::string,                   // NODE, ENV_VAR
    int,                           // MESSAGE
    std::pair<int, std::string>    // SIGNAL
  > identifier;
  std::string text;
  
  // Tracking for quoted strings
  bool is_first_quoted_string = true;
  
  // Helper methods for state validation
  bool HasValidIdentifier() const {
    switch (type) {
      case CommentType::NETWORK:
        return std::holds_alternative<std::monostate>(identifier);
      case CommentType::NODE:
      case CommentType::ENV_VAR:
        return std::holds_alternative<std::string>(identifier) && 
               !std::get<std::string>(identifier).empty();
      case CommentType::MESSAGE:
        return std::holds_alternative<int>(identifier);
      case CommentType::SIGNAL:
        return std::holds_alternative<std::pair<int, std::string>>(identifier) && 
               !std::get<std::pair<int, std::string>>(identifier).second.empty();
    }
    return false;
  }
  
  bool IsComplete() const {
    return !text.empty() && HasValidIdentifier();
  }
};

// Utility to unescape string content
std::string UnescapeString(std::string_view quoted) {
  if (quoted.size() < 2) return "";
  
  // Remove surrounding quotes
  std::string_view content = quoted.substr(1, quoted.size() - 2);
  
  std::string result;
  result.reserve(content.size());
  
  bool escaped = false;
  for (char c : content) {
    if (escaped) {
      result.push_back(c);
      escaped = false;
    } else if (c == '\\') {
      escaped = true;
    } else {
      result.push_back(c);
    }
  }
  
  return result;
}

// Actions to extract data during parsing
template<typename Rule>
struct action : pegtl::nothing<Rule> {};

template<>
struct action<grammar::quoted_string> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, CommentState& state) {
    std::string unescaped = UnescapeString(in.string());
    
    if (state.is_first_quoted_string) {
      switch (state.type) {
        case CommentType::NETWORK:
          // For NETWORK, the first string is the text
          state.text = unescaped;
          break;
        case CommentType::NODE:
        case CommentType::ENV_VAR:
          // For NODE and ENV_VAR, the first string is the identifier
          state.identifier = unescaped;
          break;
        case CommentType::MESSAGE:
          // For MESSAGE, the first string is the comment text
          state.text = unescaped;
          break;
        case CommentType::SIGNAL:
          if (std::holds_alternative<std::pair<int, std::string>>(state.identifier)) {
            // For SIGNAL, update the signal name in the pair
            auto& pair = std::get<std::pair<int, std::string>>(state.identifier);
            pair.second = unescaped;
          }
          break;
      }
      state.is_first_quoted_string = false;
    } else {
      // For all types, the non-first string is always the comment text
      state.text = unescaped;
    }
  }
};

template<>
struct action<grammar::bu_type> {
  template<typename ActionInput>
  static void apply(const ActionInput&, CommentState& state) {
    state.type = CommentType::NODE;
    state.identifier = std::string{};
    state.is_first_quoted_string = true;
  }
};

template<>
struct action<grammar::bo_type> {
  template<typename ActionInput>
  static void apply(const ActionInput&, CommentState& state) {
    state.type = CommentType::MESSAGE;
    state.identifier = 0;  // Default value, will be updated by message_id action
    state.is_first_quoted_string = true;
  }
};

template<>
struct action<grammar::sg_type> {
  template<typename ActionInput>
  static void apply(const ActionInput&, CommentState& state) {
    state.type = CommentType::SIGNAL;
    state.identifier = std::make_pair(0, "");  // Default value
    state.is_first_quoted_string = true;
  }
};

template<>
struct action<grammar::ev_type> {
  template<typename ActionInput>
  static void apply(const ActionInput&, CommentState& state) {
    state.type = CommentType::ENV_VAR;
    state.identifier = std::string{};
    state.is_first_quoted_string = true;
  }
};

template<>
struct action<grammar::message_id> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, CommentState& state) {
    try {
      int id = std::stoi(in.string());
      if (state.type == CommentType::MESSAGE) {
        state.identifier = id;
      } else if (state.type == CommentType::SIGNAL) {
        // For signal comments, create/update the pair with message ID
        if (std::holds_alternative<std::pair<int, std::string>>(state.identifier)) {
          auto& pair = std::get<std::pair<int, std::string>>(state.identifier);
          pair.first = id;
        } else {
          state.identifier = std::make_pair(id, "");
        }
      }
    } catch (const std::exception&) {
      // If conversion fails, keep the default value
    }
  }
};

std::optional<Comment> CommentParser::Parse(std::string_view input) {
  // Validate input
  if (input.empty() || input.back() != ';') {
    return std::nullopt;
  }

  CommentState state;
  pegtl::memory_input<> in(input.data(), input.size(), "comment");
  
  try {
    if (!pegtl::parse<grammar::comment_rule, action>(in, state)) {
      return std::nullopt;
    }
  } catch (const pegtl::parse_error&) {
    return std::nullopt;
  }

  if (!state.IsComplete()) {
    return std::nullopt;
  }

  Comment result;
  result.type = state.type;
  result.identifier = state.identifier;
  result.text = state.text;

  return result;
}

} // namespace parser
} // namespace dbc_parser 