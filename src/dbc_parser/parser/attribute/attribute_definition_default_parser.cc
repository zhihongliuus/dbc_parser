#include "src/dbc_parser/parser/attribute/attribute_definition_default_parser.h"

#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/analyze.hpp"
#include "src/dbc_parser/common/common_grammar.h"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for attribute definition default parsing (BA_DEF_DEF_)
namespace grammar {

// Use common grammar elements
using ws = common_grammar::ws;
using semicolon = common_grammar::semicolon;
using quoted_string = common_grammar::quoted_string;
using integer = common_grammar::integer;
using floating_point = common_grammar::floating_point;

// BA_DEF_DEF_ keyword
struct ba_def_def_keyword : pegtl::string<'B', 'A', '_', 'D', 'E', 'F', '_', 'D', 'E', 'F', '_'> {};

// Numeric value (integer or floating point)
struct numeric_value : pegtl::sor<floating_point, integer> {};

// Default value as string or number
struct default_value : pegtl::sor<quoted_string, numeric_value> {};

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

// Extract attribute name and string values from quoted strings
template<>
struct action<grammar::quoted_string> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, attribute_definition_default_state& state) {
    std::string quoted = in.string();
    
    // If we're not parsing a value, this is the attribute name
    if (!state.parsing_value) {
      state.attribute_definition_default.name = ParserBase::UnescapeString(quoted);
      state.parsing_value = true;
    } else {
      // This is a string value
      state.is_string_value = true;
      state.attribute_definition_default.default_value = ParserBase::UnescapeString(quoted);
      state.attribute_definition_default.value_type = AttributeValueType::STRING;
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
  // Validate input using ParserBase method
  if (!ValidateInput(input)) {
    return std::nullopt;
  }
  
  // Create state to collect results
  attribute_definition_default_state state;
  
  try {
    // Create input for PEGTL parser using base class method
    pegtl::memory_input<> in = CreateInput(input, "BA_DEF_DEF_");
    
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