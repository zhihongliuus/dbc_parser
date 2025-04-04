#include "src/dbc_parser/parser/attribute_value_parser.h"

#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "tao/pegtl.hpp"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for attribute value parsing (BA_)
namespace grammar {

// Basic whitespace rule
struct ws : pegtl::star<pegtl::space> {};

// BA_ keyword
struct ba_keyword : pegtl::string<'B', 'A', '_'> {};

// Object type identifiers
struct bo_keyword : pegtl::string<'B', 'O', '_'> {};
struct sg_keyword : pegtl::string<'S', 'G', '_'> {};
struct bu_keyword : pegtl::string<'B', 'U', '_'> {};
struct ev_keyword : pegtl::string<'E', 'V', '_'> {};

// Rules for quoted strings with escaping
struct escaped_char : pegtl::seq<pegtl::one<'\\'>, pegtl::any> {};
struct regular_char : pegtl::not_one<'"', '\\'> {};
struct string_content : pegtl::star<pegtl::sor<escaped_char, regular_char>> {};
struct quoted_string : pegtl::seq<pegtl::one<'"'>, string_content, pegtl::one<'"'>> {};

// Node name
struct node_name : quoted_string {};

// Signal name
struct signal_name : quoted_string {};

// Environment variable name
struct env_var_name : quoted_string {};

// Attribute name
struct attr_name : quoted_string {};

// Rules for numeric values
struct sign : pegtl::one<'+', '-'> {};
struct dot : pegtl::one<'.'> {};
struct digit : pegtl::digit {};

struct integer : pegtl::seq<
                   pegtl::opt<sign>,
                   pegtl::plus<digit>
                 > {};

struct floating_point : pegtl::seq<
                          pegtl::opt<sign>,
                          pegtl::plus<digit>,
                          dot,
                          pegtl::star<digit>
                        > {};

struct numeric_value : pegtl::sor<floating_point, integer> {};

// String value
struct string_value : quoted_string {};

// Message ID
struct message_id : pegtl::plus<digit> {};

// Attribute value
struct attr_value : pegtl::sor<string_value, numeric_value> {};

// Semicolon at the end
struct semicolon : pegtl::one<';'> {};

// Network level attribute (no object type)
struct network_attr : pegtl::seq<
                        ba_keyword,
                        ws,
                        attr_name,
                        ws,
                        attr_value,
                        ws,
                        semicolon
                      > {};

// Node attribute (BU_ node)
struct node_attr : pegtl::seq<
                     ba_keyword,
                     ws,
                     attr_name,
                     ws,
                     bu_keyword,
                     ws,
                     node_name,
                     ws,
                     attr_value,
                     ws,
                     semicolon
                   > {};

// Message attribute (BO_ message_id)
struct message_attr : pegtl::seq<
                        ba_keyword,
                        ws,
                        attr_name,
                        ws,
                        bo_keyword,
                        ws,
                        message_id,
                        ws,
                        attr_value,
                        ws,
                        semicolon
                      > {};

// Signal attribute (SG_ message_id signal)
struct signal_attr : pegtl::seq<
                       ba_keyword,
                       ws,
                       attr_name,
                       ws,
                       sg_keyword,
                       ws,
                       message_id,
                       ws,
                       signal_name,
                       ws,
                       attr_value,
                       ws,
                       semicolon
                     > {};

// Environment variable attribute (EV_ env_var)
struct env_var_attr : pegtl::seq<
                        ba_keyword,
                        ws,
                        attr_name,
                        ws,
                        ev_keyword,
                        ws,
                        env_var_name,
                        ws,
                        attr_value,
                        ws,
                        semicolon
                      > {};

// Complete rule
struct grammar : pegtl::sor<
                   signal_attr,
                   message_attr,
                   node_attr,
                   env_var_attr,
                   network_attr
                 > {};

} // namespace grammar

// Helper function to unescape string content
std::string UnescapeString(const std::string& content) {
  std::string unescaped;
  for (size_t i = 0; i < content.length(); ++i) {
    if (content[i] == '\\' && i + 1 < content.length()) {
      unescaped += content[++i];
    } else {
      unescaped += content[i];
    }
  }
  return unescaped;
}

// Structure to hold state during parsing
struct attribute_value_state {
  AttributeValue attr_value;
  int message_id = 0;
  std::string node_name;
  std::string signal_name;
  std::string env_var_name;
  std::string attr_name;
  std::string string_value;
  
  // Attribute type tracking
  enum class AttrType {
    UNKNOWN,
    NETWORK,
    NODE,
    MESSAGE,
    SIGNAL,
    ENV_VAR
  };
  
  AttrType type = AttrType::UNKNOWN;
  
  // Value type tracking
  bool has_numeric_value = false;
  bool has_float_value = false;
  bool has_string_value = false;
};

// Action rules
template<typename Rule>
struct action : pegtl::nothing<Rule> {};

// Handle quoted string content
template<>
struct action<grammar::string_content> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, attribute_value_state& state) {
    std::string content = UnescapeString(in.string());
    state.string_value = content;
  }
};

// Extract attribute name
template<>
struct action<grammar::attr_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_value_state& state) {
    state.attr_name = state.string_value;
    state.attr_value.name = state.attr_name;
  }
};

// Extract node name
template<>
struct action<grammar::node_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_value_state& state) {
    state.node_name = state.string_value;
  }
};

// Extract signal name
template<>
struct action<grammar::signal_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_value_state& state) {
    state.signal_name = state.string_value;
  }
};

// Extract environment variable name
template<>
struct action<grammar::env_var_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_value_state& state) {
    state.env_var_name = state.string_value;
  }
};

// Extract string value
template<>
struct action<grammar::string_value> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_value_state& state) {
    state.has_string_value = true;
    state.attr_value.value = state.string_value;
  }
};

// Mark as network attribute
template<>
struct action<grammar::network_attr> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_value_state& state) {
    state.type = attribute_value_state::AttrType::NETWORK;
    state.attr_value.object_type = AttributeObjectType::UNDEFINED;
    state.attr_value.object_id = std::monostate{};
  }
};

// Mark as node attribute
template<>
struct action<grammar::node_attr> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_value_state& state) {
    state.type = attribute_value_state::AttrType::NODE;
    state.attr_value.object_type = AttributeObjectType::NODE;
    state.attr_value.object_id = state.node_name;
  }
};

// Mark as message attribute
template<>
struct action<grammar::message_attr> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_value_state& state) {
    state.type = attribute_value_state::AttrType::MESSAGE;
    state.attr_value.object_type = AttributeObjectType::MESSAGE;
    state.attr_value.object_id = state.message_id;
  }
};

// Mark as signal attribute
template<>
struct action<grammar::signal_attr> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_value_state& state) {
    state.type = attribute_value_state::AttrType::SIGNAL;
    state.attr_value.object_type = AttributeObjectType::SIGNAL;
    state.attr_value.object_id = std::make_pair(state.message_id, state.signal_name);
  }
};

// Mark as environment variable attribute
template<>
struct action<grammar::env_var_attr> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_value_state& state) {
    state.type = attribute_value_state::AttrType::ENV_VAR;
    state.attr_value.object_type = AttributeObjectType::ENV_VAR;
    state.attr_value.object_id = state.env_var_name;
  }
};

// Extract message ID
template<>
struct action<grammar::message_id> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, attribute_value_state& state) {
    state.message_id = std::stoi(in.string());
  }
};

// Extract integer value
template<>
struct action<grammar::integer> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, attribute_value_state& state) {
    if (!state.has_float_value) {
      state.has_numeric_value = true;
      int value = std::stoi(in.string());
      state.attr_value.value = value;
    }
  }
};

// Extract floating point value
template<>
struct action<grammar::floating_point> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, attribute_value_state& state) {
    state.has_float_value = true;
    state.has_numeric_value = true;
    double value = std::stod(in.string());
    state.attr_value.value = value;
  }
};

std::optional<AttributeValue> AttributeValueParser::Parse(std::string_view input) {
  if (input.empty()) {
    return std::nullopt;
  }

  // Create input for PEGTL parser
  pegtl::memory_input<> in(input.data(), input.size(), "BA_");
  
  // Create state to collect results
  attribute_value_state state;
  
  try {
    // Parse input using our grammar and actions
    if (pegtl::parse<grammar::grammar, action>(in, state)) {
      return state.attr_value;
    }
  } catch (const pegtl::parse_error&) {
    // Parse error - return nullopt
    return std::nullopt;
  }
  
  return std::nullopt;
}

}  // namespace parser
}  // namespace dbc_parser 