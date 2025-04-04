#include "src/dbc_parser/parser/value_table_parser.h"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/analyze.hpp"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for VAL_TABLE_ (Value Table) parsing
namespace grammar {

// VAL_TABLE_ keyword
struct val_table_keyword : pegtl::string<'V', 'A', 'L', '_', 'T', 'A', 'B', 'L', 'E', '_'> {};

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

// String description in double quotes
struct quoted_string : pegtl::seq<
                         pegtl::one<'"'>,
                         pegtl::star<pegtl::not_one<'"'>>,
                         pegtl::one<'"'>
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

// Semicolon at the end
struct semicolon : pegtl::one<';'> {};

// Complete VAL_TABLE_ rule
struct val_table_rule : pegtl::seq<
                          pegtl::opt<pegtl::star<pegtl::space>>,
                          val_table_keyword,
                          pegtl::plus<pegtl::space>,
                          table_name,
                          value_pairs,
                          pegtl::opt<pegtl::star<pegtl::space>>,
                          semicolon,
                          pegtl::opt<pegtl::star<pegtl::space>>,
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
struct action<grammar::quoted_string> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, value_table_state& state) {
    // Extract string without quotes
    std::string quoted = in.string();
    state.current_description = quoted.substr(1, quoted.length() - 2);
    
    // Add the current value-description pair to the map
    state.value_table.values[state.current_value] = state.current_description;
  }
};

std::optional<ValueTable> ValueTableParser::Parse(std::string_view input) {
  if (input.empty()) {
    return std::nullopt;
  }

  // Create input for PEGTL parser
  pegtl::memory_input<> in(input.data(), input.size(), "VAL_TABLE_");
  
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