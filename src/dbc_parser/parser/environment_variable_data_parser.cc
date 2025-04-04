#include "src/dbc_parser/parser/environment_variable_data_parser.h"

#include <optional>
#include <string>
#include <string_view>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/parse_tree.hpp"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for ENVVAR_DATA_ (Environment Variable Data) parsing
namespace grammar {

// Basic whitespace rule
struct ws : pegtl::star<pegtl::space> {};

// ENVVAR_DATA_ keyword
struct envvar_data_keyword : pegtl::string<'E', 'N', 'V', 'V', 'A', 'R', '_', 'D', 'A', 'T', 'A', '_'> {};

// Name (identifier)
struct identifier : pegtl::plus<
                      pegtl::sor<
                        pegtl::alpha,
                        pegtl::one<'_'>,
                        pegtl::digit,
                        pegtl::one<'-'>
                      >
                    > {};

// Data size (integer)
struct integer : pegtl::plus<pegtl::digit> {};

// Colon
struct colon : pegtl::one<':'> {};

// Semicolon
struct semicolon : pegtl::one<';'> {};

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
                            pegtl::eolf
                          > {};

} // namespace grammar

// Data structure to collect parsing results
struct environment_variable_data_state {
  std::string name;
  int data_size = 0;
  
  // Flags to track parsing progress
  bool name_set = false;
  bool data_size_set = false;
};

// PEGTL actions
template<typename Rule>
struct environment_variable_data_action : pegtl::nothing<Rule> {};

// Action for extracting name
template<>
struct environment_variable_data_action<grammar::identifier> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, environment_variable_data_state& state) {
    if (!state.name_set) {
      state.name = in.string();
      state.name_set = true;
    }
  }
};

// Action for extracting data size
template<>
struct environment_variable_data_action<grammar::integer> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, environment_variable_data_state& state) {
    if (!state.data_size_set) {
      state.data_size = std::stoi(in.string());
      state.data_size_set = true;
    }
  }
};

std::optional<EnvironmentVariableData> EnvironmentVariableDataParser::Parse(std::string_view input) {
  // Create input for PEGTL parser
  pegtl::memory_input<> in(input.data(), input.size(), "ENVVAR_DATA_");
  
  // Create state to collect results
  environment_variable_data_state state;
  
  try {
    // Parse input using our grammar and actions
    if (!pegtl::parse<grammar::envvar_data_rule, environment_variable_data_action>(in, state)) {
      return std::nullopt;
    }
    
    // Verify required fields are set
    if (!state.name_set || !state.data_size_set) {
      return std::nullopt;
    }
    
    // Create and return EnvironmentVariableData object if parsing succeeded
    EnvironmentVariableData env_var_data;
    env_var_data.name = state.name;
    env_var_data.data_size = state.data_size;
    
    return env_var_data;
  } catch (const pegtl::parse_error&) {
    // Parse error - return nullopt
    return std::nullopt;
  }
}

}  // namespace parser
}  // namespace dbc_parser 