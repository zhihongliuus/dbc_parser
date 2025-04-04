#include "src/dbc_parser/parser/attribute_definition_default_parser.h"

#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/analyze.hpp"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for attribute definition default parsing (BA_DEF_DEF_)
namespace grammar {

// Basic whitespace rule
struct ws : pegtl::star<pegtl::space> {};

// BA_DEF_DEF_ keyword
struct ba_def_def_keyword : pegtl::string<'B', 'A', '_', 'D', 'E', 'F', '_', 'D', 'E', 'F', '_'> {};

// Rules for quoted strings with escaping
struct escaped_char : pegtl::seq<pegtl::one<'\\'>, pegtl::any> {};
struct regular_char : pegtl::not_one<'"', '\\'> {};
struct string_content : pegtl::star<pegtl::sor<escaped_char, regular_char>> {};
struct quoted_string : pegtl::seq<pegtl::one<'"'>, string_content, pegtl::one<'"'>> {};

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
                          dot,
                          pegtl::star<decimal_digit>
                        > {};

struct numeric_value : pegtl::sor<floating_point, integer> {};

// Default value as string or number
struct default_value : pegtl::sor<quoted_string, numeric_value> {};

// Semicolon at the end
struct semicolon : pegtl::one<';'> {};

// Complete BA_DEF_DEF_ rule
struct ba_def_def_rule : pegtl::seq<
                            ba_def_def_keyword,
                            ws,
                            quoted_string,
                            ws,
                            default_value,
                            ws,
                            semicolon
                          > {};

} // namespace grammar

// Data structure to collect parsing results
struct attribute_definition_default_state {
  AttributeDefinitionDefault attribute_definition_default;
  bool parsing_value = false;
  bool is_string_value = false;
  bool is_float_value = false;
};

// PEGTL actions
template<typename Rule>
struct action : pegtl::nothing<Rule> {};

// Extract attribute name
template<>
struct action<grammar::string_content> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, attribute_definition_default_state& state) {
    std::string content = in.string();
    
    // If we're not parsing a value, this is the attribute name
    if (!state.parsing_value) {
      state.attribute_definition_default.name = content;
      state.parsing_value = true;
    } else if (state.is_string_value) {
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
      
      state.attribute_definition_default.default_value = unescaped;
      state.attribute_definition_default.value_type = AttributeValueType::STRING;
    }
  }
};

// Set string value flag when quoted string is encountered for the value
template<>
struct action<grammar::quoted_string> {
  template<typename ActionInput>
  static void apply(const ActionInput& /*in*/, attribute_definition_default_state& state) {
    // If we're parsing a value, this is a string value
    if (state.parsing_value) {
      state.is_string_value = true;
    }
  }
};

// Extract integer value
template<>
struct action<grammar::integer> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, attribute_definition_default_state& state) {
    // If we're parsing a value, this is an integer value
    if (state.parsing_value && !state.is_float_value) {
      int value = std::stoi(in.string());
      state.attribute_definition_default.default_value = value;
      
      // Check if this is potentially an enum attribute by name
      if (state.attribute_definition_default.name.find("Enum") != std::string::npos) {
        state.attribute_definition_default.value_type = AttributeValueType::ENUM;
      } else {
        // Default to INT for integer values
        state.attribute_definition_default.value_type = AttributeValueType::INT;
      }
    }
  }
};

// Extract floating point value
template<>
struct action<grammar::floating_point> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, attribute_definition_default_state& state) {
    // If we're parsing a value, this is a floating point value
    if (state.parsing_value) {
      state.is_float_value = true;
      double value = std::stod(in.string());
      state.attribute_definition_default.default_value = value;
      state.attribute_definition_default.value_type = AttributeValueType::FLOAT;
    }
  }
};

std::optional<AttributeDefinitionDefault> AttributeDefinitionDefaultParser::Parse(std::string_view input) {
  if (input.empty()) {
    return std::nullopt;
  }

  // Create input for PEGTL parser
  pegtl::memory_input<> in(input.data(), input.size(), "BA_DEF_DEF_");
  
  // Create state to collect results
  attribute_definition_default_state state;
  
  try {
    // Parse input using our grammar and actions
    if (pegtl::parse<grammar::ba_def_def_rule, action>(in, state)) {
      return state.attribute_definition_default;
    }
  } catch (const pegtl::parse_error&) {
    // Parse error - return nullopt
    return std::nullopt;
  }
  
  return std::nullopt;
}

}  // namespace parser
}  // namespace dbc_parser 