#include "src/dbc_parser/parser/attribute_value_parser.h"

#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "tao/pegtl.hpp"
#include "src/dbc_parser/parser/common_grammar.h"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for attribute value parsing (BA_)
namespace grammar {

// Use common grammar elements
using ws = common_grammar::ws;
using semicolon = common_grammar::semicolon;
using quoted_string = common_grammar::quoted_string;
using integer = common_grammar::integer;
using floating_point = common_grammar::floating_point;
using digit = common_grammar::digit;

// Use common keywords
using bo_keyword = common_grammar::bo_keyword;
using sg_keyword = common_grammar::sg_keyword;
using bu_keyword = common_grammar::bu_keyword;
using ev_keyword = common_grammar::ev_keyword;

// BA_ keyword
struct ba_keyword : pegtl::string<'B', 'A', '_'> {};

// Node name, signal name, env var name, and attribute name
struct node_name : common_grammar::quoted_string {};
struct signal_name : common_grammar::quoted_string {};
struct env_var_name : common_grammar::quoted_string {};
struct attr_name : common_grammar::quoted_string {};

// Message ID
struct message_id : pegtl::plus<digit> {};

// Attribute value
struct numeric_value : pegtl::sor<floating_point, integer> {};
struct string_value : common_grammar::quoted_string {};
struct attr_value : pegtl::sor<string_value, numeric_value> {};

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

// Structure to hold state during parsing
struct attribute_value_state {
  AttributeValue attr_value;
  int message_id = 0;
  std::string node_name;
  std::string signal_name;
  std::string env_var_name;
  std::string attr_name;
  std::string string_value;
  
  // Attribute type tracking using the common AttributeObjectType
  AttributeObjectType type = AttributeObjectType::UNDEFINED;
  
  // Value type tracking
  bool has_numeric_value = false;
  bool has_float_value = false;
  bool has_string_value = false;
};

// Action rules
template<typename Rule>
struct action : pegtl::nothing<Rule> {};

// Extract attribute name
template<>
struct action<grammar::attr_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, attribute_value_state& state) {
    state.attr_name = ParserBase::UnescapeString(in.string());
    state.attr_value.name = state.attr_name;
  }
};

// Extract node name
template<>
struct action<grammar::node_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, attribute_value_state& state) {
    state.node_name = ParserBase::UnescapeString(in.string());
  }
};

// Extract signal name
template<>
struct action<grammar::signal_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, attribute_value_state& state) {
    state.signal_name = ParserBase::UnescapeString(in.string());
  }
};

// Extract environment variable name
template<>
struct action<grammar::env_var_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, attribute_value_state& state) {
    state.env_var_name = ParserBase::UnescapeString(in.string());
  }
};

// Extract string value
template<>
struct action<grammar::string_value> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, attribute_value_state& state) {
    state.has_string_value = true;
    state.attr_value.value = ParserBase::UnescapeString(in.string());
  }
};

// Mark as network attribute
template<>
struct action<grammar::network_attr> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_value_state& state) {
    state.type = AttributeObjectType::NETWORK;
    state.attr_value.object_type = AttributeObjectType::UNDEFINED;
    state.attr_value.object_id = std::monostate{};
  }
};

// Mark as node attribute
template<>
struct action<grammar::node_attr> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_value_state& state) {
    state.type = AttributeObjectType::NODE;
    state.attr_value.object_type = AttributeObjectType::NODE;
    state.attr_value.object_id = state.node_name;
  }
};

// Mark as message attribute
template<>
struct action<grammar::message_attr> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_value_state& state) {
    state.type = AttributeObjectType::MESSAGE;
    state.attr_value.object_type = AttributeObjectType::MESSAGE;
    state.attr_value.object_id = state.message_id;
  }
};

// Mark as signal attribute
template<>
struct action<grammar::signal_attr> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_value_state& state) {
    state.type = AttributeObjectType::SIGNAL;
    state.attr_value.object_type = AttributeObjectType::SIGNAL;
    state.attr_value.object_id = std::make_pair(state.message_id, state.signal_name);
  }
};

// Mark as environment variable attribute
template<>
struct action<grammar::env_var_attr> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_value_state& state) {
    state.type = AttributeObjectType::ENV_VAR;
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
  // Validate input using ParserBase method
  if (!ValidateInput(input)) {
    return std::nullopt;
  }
  
  // Create state to collect results
  attribute_value_state state;
  
  try {
    // Create input for PEGTL parser using base class method
    pegtl::memory_input<> in = CreateInput(input, "BA_");
    
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