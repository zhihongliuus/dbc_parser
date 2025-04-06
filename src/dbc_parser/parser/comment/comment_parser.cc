#include "dbc_parser/parser/comment/comment_parser.h"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>

#include "dbc_parser/common/common_grammar.h"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for parsing comments in DBC files
namespace grammar {

// Keywords for comments
struct cm_prefix : pegtl::string<'C', 'M', '_'> {};

// Object type indicators
struct bu_type : pegtl::string<'B', 'U', '_'> {};
struct bo_type : pegtl::string<'B', 'O', '_'> {};
struct sg_type : pegtl::string<'S', 'G', '_'> {};
struct ev_type : pegtl::string<'E', 'V', '_'> {};

// Different comment types
struct network_comment : pegtl::seq<
    cm_prefix, common_grammar::ws,
    common_grammar::quoted_string, common_grammar::ws,
    common_grammar::semicolon
> {};

struct node_comment : pegtl::seq<
    cm_prefix, common_grammar::ws,
    bu_type, common_grammar::ws,
    pegtl::sor<common_grammar::quoted_identifier, common_grammar::unquoted_identifier>, common_grammar::ws,
    common_grammar::quoted_string, common_grammar::ws,
    common_grammar::semicolon
> {};

struct message_comment : pegtl::seq<
    cm_prefix, common_grammar::ws,
    bo_type, common_grammar::ws,
    common_grammar::message_id, common_grammar::ws,
    common_grammar::quoted_string, common_grammar::ws,
    common_grammar::semicolon
> {};

struct signal_comment : pegtl::seq<
    cm_prefix, common_grammar::ws,
    sg_type, common_grammar::ws,
    common_grammar::message_id, common_grammar::ws,
    pegtl::sor<common_grammar::quoted_identifier, common_grammar::unquoted_identifier>, common_grammar::ws,
    common_grammar::quoted_string, common_grammar::ws,
    common_grammar::semicolon
> {};

struct env_var_comment : pegtl::seq<
    cm_prefix, common_grammar::ws,
    ev_type, common_grammar::ws,
    pegtl::sor<common_grammar::quoted_identifier, common_grammar::unquoted_identifier>, common_grammar::ws,
    common_grammar::quoted_string, common_grammar::ws,
    common_grammar::semicolon
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

// Actions to extract data during parsing
template<typename Rule>
struct action : pegtl::nothing<Rule> {};

template<>
struct action<common_grammar::quoted_string> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, CommentState& state) noexcept {
    std::string unescaped = ParserBase::UnescapeString(in.string());
    
    // For all types, the quoted string is the comment text
    state.text = unescaped;
  }
};

template<>
struct action<grammar::bu_type> {
  template<typename ActionInput>
  static void apply(const ActionInput&, CommentState& state) noexcept {
    state.type = CommentType::NODE;
    state.identifier = std::string{};
  }
};

template<>
struct action<grammar::bo_type> {
  template<typename ActionInput>
  static void apply(const ActionInput&, CommentState& state) noexcept {
    state.type = CommentType::MESSAGE;
    state.identifier = 0;  // Default value, will be updated by message_id action
  }
};

template<>
struct action<grammar::sg_type> {
  template<typename ActionInput>
  static void apply(const ActionInput&, CommentState& state) noexcept {
    state.type = CommentType::SIGNAL;
    state.identifier = std::make_pair(0, "");  // Default value
  }
};

template<>
struct action<grammar::ev_type> {
  template<typename ActionInput>
  static void apply(const ActionInput&, CommentState& state) noexcept {
    state.type = CommentType::ENV_VAR;
    state.identifier = std::string{};
  }
};

template<>
struct action<common_grammar::message_id> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, CommentState& state) noexcept {
    try {
      int id = std::stoi(in.string());
      
      if (state.type == CommentType::MESSAGE) {
        state.identifier = id;
      } else if (state.type == CommentType::SIGNAL && 
                 std::holds_alternative<std::pair<int, std::string>>(state.identifier)) {
        auto& pair = std::get<std::pair<int, std::string>>(state.identifier);
        pair.first = id;
      }
    } catch (const std::exception&) {
      // Invalid message ID - we'll set a default value
      if (state.type == CommentType::MESSAGE) {
        state.identifier = 0;
      } else if (state.type == CommentType::SIGNAL && 
                 std::holds_alternative<std::pair<int, std::string>>(state.identifier)) {
        auto& pair = std::get<std::pair<int, std::string>>(state.identifier);
        pair.first = 0;
      }
    }
  }
};

template<>
struct action<common_grammar::quoted_identifier> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, CommentState& state) noexcept {
    std::string unescaped = ParserBase::UnescapeString(in.string());
    
    // Store the unquoted string based on the comment type
    if (state.type == CommentType::NODE) {
      state.identifier = unescaped;
    } else if (state.type == CommentType::ENV_VAR) {
      state.identifier = unescaped;
    } else if (state.type == CommentType::SIGNAL && 
               std::holds_alternative<std::pair<int, std::string>>(state.identifier)) {
      auto& pair = std::get<std::pair<int, std::string>>(state.identifier);
      pair.second = unescaped;
    }
  }
};

template<>
struct action<common_grammar::unquoted_identifier> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, CommentState& state) noexcept {
    // Store the identifier string directly based on the comment type
    if (state.type == CommentType::NODE) {
      state.identifier = in.string();
    } else if (state.type == CommentType::ENV_VAR) {
      state.identifier = in.string();
    } else if (state.type == CommentType::SIGNAL && 
               std::holds_alternative<std::pair<int, std::string>>(state.identifier)) {
      auto& pair = std::get<std::pair<int, std::string>>(state.identifier);
      pair.second = in.string();
    }
  }
};

std::optional<Comment> CommentParser::Parse(std::string_view input) {
  // Validate input
  if (!ValidateInput(input) || input.back() != ';') {
    return std::nullopt;
  }

  CommentState state;
  pegtl::memory_input<> in = CreateInput(input, "comment");
  
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