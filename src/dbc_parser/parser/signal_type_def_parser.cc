#include "src/dbc_parser/parser/signal_type_def_parser.h"

#include <iostream>
#include <optional>
#include <string>
#include <string_view>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/parse_tree.hpp"
#include "src/dbc_parser/parser/common_grammar.h"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for SIG_TYPE_DEF_ (Signal Type Definition) parsing
namespace grammar {

// Use common grammar elements
using ws = common_grammar::ws;
using semicolon = common_grammar::semicolon;
using comma = common_grammar::comma;
using colon = common_grammar::colon;
using identifier = common_grammar::identifier;
using integer = common_grammar::integer;
using floating_point = common_grammar::floating_point;
using quoted_string = common_grammar::quoted_string;

// SIG_TYPE_DEF_ keyword
struct sig_type_def_keyword : pegtl::string<'S', 'I', 'G', '_', 'T', 'Y', 'P', 'E', '_', 'D', 'E', 'F', '_'> {};

// Value type ('+' for unsigned, '-' for signed, etc.)
struct value_type : pegtl::plus<pegtl::not_one<' ', '\t', ',', ';'>> {};

struct unquoted_string : pegtl::plus<pegtl::not_one<' ', '\t', ',', ';'>> {};

struct unit_string : pegtl::sor<quoted_string, unquoted_string> {};

// Decimal value (for factor, offset, min, max, default)
struct decimal : pegtl::seq<
                   pegtl::opt<pegtl::one<'-'>>,
                   pegtl::plus<pegtl::digit>,
                   pegtl::opt<
                     pegtl::seq<
                       pegtl::one<'.'>,
                       pegtl::star<pegtl::digit>
                     >
                   >
                 > {};

// Optional whitespace and comma
struct opt_ws_comma : pegtl::seq<ws, comma, ws> {};

// Complete SIG_TYPE_DEF_ rule
struct sig_type_def_rule : pegtl::seq<
                             ws,
                             sig_type_def_keyword,
                             ws,
                             identifier,  // signal type name
                             ws,
                             colon,
                             ws,
                             integer,     // size in bits
                             opt_ws_comma,
                             integer,     // byte order
                             opt_ws_comma,
                             value_type,  // value type
                             opt_ws_comma,
                             decimal,     // factor
                             opt_ws_comma,
                             decimal,     // offset
                             opt_ws_comma,
                             decimal,     // minimum
                             opt_ws_comma,
                             decimal,     // maximum
                             opt_ws_comma,
                             unit_string, // unit
                             opt_ws_comma,
                             decimal,     // default value
                             opt_ws_comma,
                             pegtl::opt<identifier>, // value table (optional)
                             ws,
                             semicolon,
                             pegtl::eolf
                           > {};

} // namespace grammar

// Data structure to collect parsing results
struct signal_type_def_state {
  std::string name;
  int size = 0;
  int byte_order = 0;
  std::string value_type;
  double factor = 1.0;
  double offset = 0.0;
  double minimum = 0.0;
  double maximum = 0.0;
  std::string unit;
  double default_value = 0.0;
  std::string value_table;
  
  // Flags to track parsing progress
  bool name_set = false;
  bool size_set = false;
  bool byte_order_set = false;
  bool value_type_set = false;
  bool factor_set = false;
  bool offset_set = false;
  bool minimum_set = false;
  bool maximum_set = false;
  bool unit_set = false;
  bool default_value_set = false;
  bool value_table_set = false;
};

// PEGTL actions
template<typename Rule>
struct signal_type_def_action : pegtl::nothing<Rule> {};

// Action for extracting name
template<>
struct signal_type_def_action<grammar::identifier> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_type_def_state& state) {
    if (!state.name_set) {
      state.name = in.string();
      state.name_set = true;
    } else if (!state.value_table_set) {
      // Only set value_table if non-empty
      std::string value = in.string();
      if (!value.empty()) {
        state.value_table = value;
        state.value_table_set = true;
      }
    }
  }
};

// Action for extracting size and byte order
template<>
struct signal_type_def_action<grammar::integer> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_type_def_state& state) {
    if (!state.size_set) {
      state.size = std::stoi(in.string());
      state.size_set = true;
    } else if (!state.byte_order_set) {
      state.byte_order = std::stoi(in.string());
      state.byte_order_set = true;
    }
  }
};

// Action for extracting value type
template<>
struct signal_type_def_action<grammar::value_type> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_type_def_state& state) {
    if (!state.value_type_set) {
      state.value_type = in.string();
      state.value_type_set = true;
    }
  }
};

// Action for extracting numeric values (factor, offset, min, max, default)
template<>
struct signal_type_def_action<grammar::decimal> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_type_def_state& state) {
    if (!state.factor_set) {
      state.factor = std::stod(in.string());
      state.factor_set = true;
    } else if (!state.offset_set) {
      state.offset = std::stod(in.string());
      state.offset_set = true;
    } else if (!state.minimum_set) {
      state.minimum = std::stod(in.string());
      state.minimum_set = true;
    } else if (!state.maximum_set) {
      state.maximum = std::stod(in.string());
      state.maximum_set = true;
    } else if (!state.default_value_set) {
      state.default_value = std::stod(in.string());
      state.default_value_set = true;
    }
  }
};

// Action for extracting unit string
template<>
struct signal_type_def_action<grammar::quoted_string> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_type_def_state& state) {
    if (!state.unit_set) {
      // Use ParserBase method to unescape the string
      state.unit = ParserBase::UnescapeString(in.string());
      state.unit_set = true;
    }
  }
};

template<>
struct signal_type_def_action<grammar::unquoted_string> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_type_def_state& state) {
    // Only use unquoted_string action for unit if value_type is already set
    // and unit is not set
    if (state.value_type_set && !state.unit_set && in.string() != state.value_type) {
      state.unit = in.string();
      state.unit_set = true;
    }
  }
};

std::optional<SignalTypeDef> SignalTypeDefParser::Parse(std::string_view input) {
  // Validate input using ParserBase method
  if (!ValidateInput(input)) {
    return std::nullopt;
  }
  
  // Create state to collect results
  signal_type_def_state state;
  
  try {
    // Create input for PEGTL parser using base class method
    pegtl::memory_input<> in = CreateInput(input, "SIG_TYPE_DEF_");
    
    // Parse input using our grammar and actions
    if (!pegtl::parse<grammar::sig_type_def_rule, signal_type_def_action>(in, state)) {
      return std::nullopt;
    }
    
    // Verify required fields are set
    if (!state.name_set || !state.size_set || !state.byte_order_set || 
        !state.value_type_set || !state.factor_set || !state.offset_set || 
        !state.minimum_set || !state.maximum_set || !state.unit_set || 
        !state.default_value_set) {
      return std::nullopt;
    }
    
    // Create and return SignalTypeDef object if parsing succeeded
    SignalTypeDef sig_type_def;
    sig_type_def.name = state.name;
    sig_type_def.size = state.size;
    sig_type_def.byte_order = state.byte_order;
    sig_type_def.value_type = state.value_type;
    sig_type_def.factor = state.factor;
    sig_type_def.offset = state.offset;
    sig_type_def.minimum = state.minimum;
    sig_type_def.maximum = state.maximum;
    sig_type_def.unit = state.unit;
    sig_type_def.default_value = state.default_value;
    sig_type_def.value_table = state.value_table;
    
    return sig_type_def;
  } catch (const pegtl::parse_error&) {
    // Parse error - return nullopt
    return std::nullopt;
  }
}

}  // namespace parser
}  // namespace dbc_parser 