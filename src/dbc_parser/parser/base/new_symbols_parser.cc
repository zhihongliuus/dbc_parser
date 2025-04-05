#include "src/dbc_parser/parser/base/new_symbols_parser.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include <sstream>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/analyze.hpp"

#include "src/dbc_parser/common/common_grammar.h"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for NS_ (New Symbols) parsing
namespace grammar {

// Use common grammar elements
using ws = common_grammar::ws;
using ns_keyword = common_grammar::ns_keyword;
using colon = common_grammar::colon;

// A symbol (e.g., CM_, BA_, SG_, etc.)
struct symbol : pegtl::seq<
                  pegtl::plus<pegtl::alpha>,
                  pegtl::one<'_'>
                > {};

// Multiple symbols separated by whitespace
struct symbols : pegtl::star<
                   pegtl::seq<
                     pegtl::plus<pegtl::space>,
                     symbol
                   >
                 > {};

// Complete NS_ rule
struct ns_rule : pegtl::seq<
                   ws,
                   ns_keyword,
                   ws,
                   colon,
                   pegtl::opt<symbols>,
                   ws,
                   pegtl::eof
                 > {};

} // namespace grammar

// Data structure to collect parsing results
struct symbols_state {
  std::vector<std::string> symbols;
};

// PEGTL actions
template<typename Rule>
struct symbols_action : pegtl::nothing<Rule> {};

// Action for extracting symbols
template<>
struct symbols_action<grammar::symbol> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, symbols_state& state) {
    state.symbols.push_back(in.string());
  }
};

std::optional<NewSymbols> NewSymbolsParser::Parse(std::string_view input) {
  // Validate input
  if (!ValidateInput(input)) {
    return std::nullopt;
  }
  
  // Create input for PEGTL parser
  pegtl::memory_input<> in = CreateInput(input, "NS_");
  
  // Create state to collect results
  symbols_state state;
  
  try {
    // Parse input using our grammar and actions
    if (pegtl::parse<grammar::ns_rule, symbols_action>(in, state)) {
      NewSymbols new_symbols;
      new_symbols.symbols = state.symbols;
      return new_symbols;
    }
  } catch (const pegtl::parse_error&) {
    // Parse error - return nullopt
    return std::nullopt;
  }
  
  return std::nullopt;
}

}  // namespace parser
}  // namespace dbc_parser 