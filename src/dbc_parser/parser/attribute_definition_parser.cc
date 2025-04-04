#include "src/dbc_parser/parser/attribute_definition_parser.h"

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <variant>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/analyze.hpp"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for attribute definition parsing (BA_DEF_)
namespace grammar {

// Basic whitespace rule
struct ws : pegtl::star<pegtl::space> {};

// BA_DEF_ keyword
struct ba_def_keyword : pegtl::string<'B', 'A', '_', 'D', 'E', 'F', '_'> {};

// Object type identifiers
struct bo_keyword : pegtl::string<'B', 'O', '_'> {};
struct sg_keyword : pegtl::string<'S', 'G', '_'> {};
struct bu_keyword : pegtl::string<'B', 'U', '_'> {};
struct ev_keyword : pegtl::string<'E', 'V', '_'> {};

// Object type
struct object_type : pegtl::sor<
                       bo_keyword,
                       sg_keyword,
                       bu_keyword,
                       ev_keyword
                     > {};

// Attribute value types
struct int_keyword : pegtl::string<'I', 'N', 'T'> {};
struct hex_keyword : pegtl::string<'H', 'E', 'X'> {};
struct float_keyword : pegtl::string<'F', 'L', 'O', 'A', 'T'> {};
struct string_keyword : pegtl::string<'S', 'T', 'R', 'I', 'N', 'G'> {};
struct enum_keyword : pegtl::string<'E', 'N', 'U', 'M'> {};

struct value_type : pegtl::sor<
                      int_keyword,
                      hex_keyword,
                      float_keyword,
                      string_keyword,
                      enum_keyword
                    > {};

// Rules for quoted strings with escaping
struct escaped_char : pegtl::seq<pegtl::one<'\\'>, pegtl::any> {};
struct regular_char : pegtl::not_one<'"', '\\'> {};
struct string_content : pegtl::star<pegtl::sor<escaped_char, regular_char>> {};
struct quoted_string : pegtl::seq<pegtl::one<'"'>, string_content, pegtl::one<'"'>> {};

// Comma for separating enum values
struct comma : pegtl::one<','> {};

// Rules for numeric values
struct decimal_digit : pegtl::range<'0', '9'> {};
struct sign : pegtl::one<'+', '-'> {};
struct dot : pegtl::one<'.'> {};

struct integer : pegtl::seq<
                   pegtl::opt<sign>,
                   pegtl::plus<decimal_digit>
                 > {};

struct floating_point : pegtl::seq<
                          pegtl::opt<sign>,
                          pegtl::plus<decimal_digit>,
                          pegtl::opt<
                            pegtl::seq<
                              dot,
                              pegtl::star<decimal_digit>
                            >
                          >
                        > {};

struct numeric_value : pegtl::sor<floating_point, integer> {};

// Semicolon at the end
struct semicolon : pegtl::one<';'> {};

// Enum values list
struct enum_values : pegtl::list<quoted_string, comma> {};

// Integer or float attribute with range
struct numeric_attribute : pegtl::seq<
                             pegtl::sor<int_keyword, hex_keyword, float_keyword>,
                             ws,
                             numeric_value,
                             ws,
                             numeric_value
                           > {};

// String attribute
struct string_attribute : string_keyword {};

// Enum attribute
struct enum_attribute : pegtl::seq<
                          enum_keyword,
                          ws,
                          enum_values
                        > {};

// Attribute value type with optional parameters
struct attribute_value_type : pegtl::sor<
                                numeric_attribute,
                                string_attribute,
                                enum_attribute
                              > {};

// Network level attribute definition (no object type specified)
struct network_attr_def : pegtl::seq<
                            ba_def_keyword,
                            ws,
                            quoted_string,
                            ws,
                            attribute_value_type,
                            ws,
                            semicolon
                          > {};

// Object specific attribute definition
struct object_attr_def : pegtl::seq<
                           ba_def_keyword,
                           ws,
                           object_type,
                           ws,
                           quoted_string,
                           ws,
                           attribute_value_type,
                           ws,
                           semicolon
                         > {};

// Complete BA_DEF_ rule
struct ba_def_rule : pegtl::sor<network_attr_def, object_attr_def> {};

} // namespace grammar

// Data structure to collect parsing results
struct attribute_definition_state {
  AttributeDefinition attribute_definition;
  bool is_enum = false;
  bool is_numeric = false;
  AttributeValueType value_type;
};

// Map from PEGTL object type keywords to AttributeObjectType
const std::map<std::string, AttributeObjectType> kObjectTypeMap = {
  {"BO_", AttributeObjectType::MESSAGE},
  {"SG_", AttributeObjectType::SIGNAL},
  {"BU_", AttributeObjectType::NODE},
  {"EV_", AttributeObjectType::ENV_VAR}
};

// PEGTL actions
template<typename Rule>
struct action : pegtl::nothing<Rule> {};

// Extract object type
template<>
struct action<grammar::object_type> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, attribute_definition_state& state) {
    std::string object_type_str = in.string();
    state.attribute_definition.object_type = kObjectTypeMap.at(object_type_str);
  }
};

// When no object type is specified, it's a network-level attribute
template<>
struct action<grammar::network_attr_def> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_definition_state& state) {
    state.attribute_definition.object_type = AttributeObjectType::UNDEFINED;
  }
};

// Extract attribute name
template<>
struct action<grammar::string_content> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, attribute_definition_state& state) {
    // If this is the first string_content we've seen and the name is empty, it's the attribute name
    if (state.attribute_definition.name.empty()) {
      state.attribute_definition.name = in.string();
    } else if (state.is_enum) {
      // This is an enum value
      std::string enum_value = in.string();
      
      // Unescape any escaped characters
      std::string unescaped;
      for (size_t i = 0; i < enum_value.length(); ++i) {
        if (enum_value[i] == '\\' && i + 1 < enum_value.length()) {
          // Skip backslash and add the escaped character
          unescaped += enum_value[++i];
        } else {
          unescaped += enum_value[i];
        }
      }
      
      state.attribute_definition.enum_values.push_back(unescaped);
    }
  }
};

// Extract value type
template<>
struct action<grammar::int_keyword> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_definition_state& state) {
    state.attribute_definition.value_type = AttributeValueType::INT;
    state.value_type = AttributeValueType::INT;
    state.is_numeric = true;
  }
};

template<>
struct action<grammar::hex_keyword> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_definition_state& state) {
    state.attribute_definition.value_type = AttributeValueType::HEX;
    state.value_type = AttributeValueType::HEX;
    state.is_numeric = true;
  }
};

template<>
struct action<grammar::float_keyword> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_definition_state& state) {
    state.attribute_definition.value_type = AttributeValueType::FLOAT;
    state.value_type = AttributeValueType::FLOAT;
    state.is_numeric = true;
  }
};

template<>
struct action<grammar::string_keyword> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_definition_state& state) {
    state.attribute_definition.value_type = AttributeValueType::STRING;
    state.value_type = AttributeValueType::STRING;
  }
};

template<>
struct action<grammar::string_attribute> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_definition_state& state) {
    state.attribute_definition.value_type = AttributeValueType::STRING;
    state.value_type = AttributeValueType::STRING;
  }
};

template<>
struct action<grammar::enum_keyword> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_definition_state& state) {
    state.attribute_definition.value_type = AttributeValueType::ENUM;
    state.value_type = AttributeValueType::ENUM;
    state.is_enum = true;
  }
};

// Extract min/max values for numeric types
template<>
struct action<grammar::numeric_value> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, attribute_definition_state& state) {
    if (state.is_numeric) {
      double value = std::stod(in.string());
      
      // If min_value is not set, this is the min_value, otherwise it's the max_value
      if (!state.attribute_definition.min_value.has_value()) {
        state.attribute_definition.min_value = value;
      } else if (!state.attribute_definition.max_value.has_value()) {
        state.attribute_definition.max_value = value;
      }
    }
  }
};

std::optional<AttributeDefinition> AttributeDefinitionParser::Parse(std::string_view input) {
  if (input.empty()) {
    return std::nullopt;
  }

  // Create input for PEGTL parser
  pegtl::memory_input<> in(input.data(), input.size(), "BA_DEF_");
  
  // Create state to collect results
  attribute_definition_state state;
  
  try {
    // Parse input using our grammar and actions
    if (pegtl::parse<grammar::ba_def_rule, action>(in, state)) {
      // Set a default value placeholder (it will be set by BA_DEF_DEF_ later)
      if (state.attribute_definition.value_type == AttributeValueType::INT ||
          state.attribute_definition.value_type == AttributeValueType::HEX) {
        state.attribute_definition.default_value = 0;
      } else if (state.attribute_definition.value_type == AttributeValueType::FLOAT) {
        state.attribute_definition.default_value = 0.0;
      } else {
        state.attribute_definition.default_value = std::string("");
      }
      
      return state.attribute_definition;
    }
  } catch (const pegtl::parse_error&) {
    // Parse error - return nullopt
    return std::nullopt;
  }
  
  return std::nullopt;
}

}  // namespace parser
}  // namespace dbc_parser 