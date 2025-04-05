#include "src/dbc_parser/parser/environment/environment_variable_data_parser.h"

#include <optional>
#include <string>
#include <string_view>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/parse_tree.hpp"

#include "src/dbc_parser/common/common_grammar.h"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for ENVVAR_DATA_ (Environment Variable Data) parsing
namespace grammar {

// Use common grammar elements
using ws = common_grammar::ws;
using integer = common_grammar::integer;
using colon = common_grammar::colon;
using semicolon = common_grammar::semicolon;
using envvar_data_keyword = common_grammar::envvar_data_keyword;

// Name (identifier)
struct identifier : pegtl::plus<
                      pegtl::sor<
                        pegtl::alpha,
                        pegtl::one<'_'>,
                        pegtl::digit,
                        pegtl::one<'-'>
                      >
                    > {};

// Complete ENVVAR_DATA_ rule
struct envvar_data_rule : pegtl::seq<
                            ws,
                            envvar_data_keyword,
                            ws,
                            identifier,  // env var name
                            ws,
                            colon,
                            ws,
                            integer,     // data size
                            ws,
                            semicolon,
                            pegtl::eof
                          > {};

} // namespace grammar

// Data structure to collect parsing results
struct environment_variable_data_state {
  std::string name;
  std::string data;
  
  // Flags to track parsing progress
  bool name_set = false;
};

// PEGTL actions
template<typename Rule>
struct environment_variable_data_action : pegtl::nothing<Rule> {};

// Action for extracting name
template<>
struct environment_variable_data_action<grammar::identifier> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, environment_variable_data_state& state) noexcept {
    if (!state.name_set) {
      state.name = in.string();
      state.name_set = true;
    }
  }
};

std::optional<EnvironmentVariableData> EnvironmentVariableDataParser::Parse(std::string_view input) {
  // Validate input using base class method
  if (!ValidateInput(input)) {
    return std::nullopt;
  }
  
  // Create input for PEGTL parser using base class method
  pegtl::memory_input<> in = CreateInput(input, "ENVVAR_DATA_");
  
  // Create state to collect results
  environment_variable_data_state state;
  
  try {
    // Parse input using our grammar and actions
    if (!pegtl::parse<grammar::envvar_data_rule, environment_variable_data_action>(in, state)) {
      return std::nullopt;
    }
    
    // Verify required fields are set
    if (!state.name_set) {
      return std::nullopt;
    }
    
    // Create and return EnvironmentVariableData object if parsing succeeded
    EnvironmentVariableData env_var_data;
    env_var_data.name = state.name;
    env_var_data.data = input.data(); // Store the full input as data
    
    return env_var_data;
  } catch (const pegtl::parse_error&) {
    // Parse error - return nullopt
    return std::nullopt;
  }
}

}  // namespace parser
}  // namespace dbc_parser 