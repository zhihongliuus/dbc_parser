#include "src/dbc_parser/parser/value/value_table_parser.h"

#include <map>
#include <optional>
#include <string>
#include <string_view>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/analyze.hpp"

#include "src/dbc_parser/common/common_grammar.h"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for VAL_TABLE_ (Value Table) parsing
namespace grammar {
// Use common grammar elements
using ws = common_grammar::ws;
using semicolon = common_grammar::semicolon;
using quoted_string = common_grammar::quoted_string;
using val_table_keyword = common_grammar::val_table_keyword;

// Table name
struct table_name_char : pegtl::sor<
                          pegtl::alpha,
                          pegtl::digit,
                          pegtl::one<'_'>,
                          pegtl::one<'-'>
                        > {};

struct table_name : pegtl::plus<table_name_char> {};

// Integer value
struct integer : pegtl::seq<
                   pegtl::opt<pegtl::one<'-'>>,
                   pegtl::plus<pegtl::digit>
                 > {};

// Value-description pair
struct value_pair : pegtl::seq<
                      integer,
                      pegtl::plus<pegtl::space>,
                      quoted_string
                    > {};

// Multiple value-description pairs
struct value_pairs : pegtl::star<
                       pegtl::seq<
                         pegtl::plus<pegtl::space>,
                         value_pair
                       >
                     > {};

// Complete VAL_TABLE_ rule
struct val_table_rule : pegtl::seq<
                          ws,
                          val_table_keyword,
                          pegtl::plus<pegtl::space>,
                          table_name,
                          value_pairs,
                          ws,
                          semicolon,
                          ws,
                          pegtl::eof
                        > {};

} // namespace grammar

// Data structure to collect parsing results
struct value_table_state {
  ValueTable value_table;
  int current_value;
  std::string current_description;
};

// PEGTL actions
template<typename Rule>
struct action : pegtl::nothing<Rule> {};

// Action for extracting table name
template<>
struct action<grammar::table_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, value_table_state& state) {
    state.value_table.name = in.string();
  }
};

// Action for extracting integer value
template<>
struct action<grammar::integer> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, value_table_state& state) {
    state.current_value = std::stoi(in.string());
  }
};

// Action for extracting string description
template<>
struct action<common_grammar::quoted_string> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, value_table_state& state) {
    // Extract string without quotes using ParserBase method
    state.current_description = ParserBase::UnescapeString(in.string());
    
    // Add the current value-description pair to the map
    state.value_table.values[state.current_value] = state.current_description;
  }
};

std::optional<ValueTable> ValueTableParser::Parse(std::string_view input) {
  // Validate input using base class method
  if (!ValidateInput(input)) {
    return std::nullopt;
  }

  // Create input for PEGTL parser using base class method
  pegtl::memory_input<> in = CreateInput(input, "VAL_TABLE_");
  
  // Create state to collect results
  value_table_state state;
  
  try {
    // Parse input using our grammar and actions
    if (pegtl::parse<grammar::val_table_rule, action>(in, state)) {
      return state.value_table;
    }
  } catch (const pegtl::parse_error&) {
    // Parse error - return nullopt
    return std::nullopt;
  }
  
  return std::nullopt;
}
} // namespace parser
} // namespace dbc_parser 