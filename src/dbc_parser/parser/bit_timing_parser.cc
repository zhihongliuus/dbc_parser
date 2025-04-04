#include "src/dbc_parser/parser/bit_timing_parser.h"

#include <optional>
#include <string>
#include <string_view>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/parse_tree.hpp"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for BS_ (Bit Timing) parsing
namespace grammar {

// Basic whitespace rule
struct ws : pegtl::star<pegtl::space> {};

// BS_ keyword
struct bs_keyword : pegtl::string<'B', 'S', '_'> {};

// Colon separator
struct colon : pegtl::one<':'> {};

// Integer (for baudrate)
struct integer : pegtl::plus<pegtl::digit> {};

// Floating point number (for BTR1/BTR2)
struct decimal : pegtl::seq<pegtl::opt<pegtl::one<'-'>>, 
                            pegtl::plus<pegtl::digit>,
                            pegtl::opt<
                              pegtl::seq<
                                pegtl::one<'.'>,
                                pegtl::star<pegtl::digit>
                              >
                            >> {};

// Complete BS_ rule
struct bs_rule : pegtl::seq<
                   ws,
                   bs_keyword,
                   ws,
                   colon,
                   ws,
                   integer,
                   ws,
                   decimal,
                   ws,
                   pegtl::eof
                 > {};

} // namespace grammar

// Data structure to collect parsing results
struct bit_timing_state {
  int baudrate = 0;
  double btr1_btr2 = 0.0;
  bool baudrate_set = false;
  bool btr1_btr2_set = false;
};

// PEGTL actions
template<typename Rule>
struct bit_timing_action : pegtl::nothing<Rule> {};

// Action for extracting baudrate
template<>
struct bit_timing_action<grammar::integer> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, bit_timing_state& state) {
    if (!state.baudrate_set) {
      state.baudrate = std::stoi(in.string());
      state.baudrate_set = true;
    }
  }
};

// Action for extracting BTR1/BTR2
template<>
struct bit_timing_action<grammar::decimal> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, bit_timing_state& state) {
    if (!state.btr1_btr2_set) {
      state.btr1_btr2 = std::stod(in.string());
      state.btr1_btr2_set = true;
    }
  }
};

std::optional<BitTiming> BitTimingParser::Parse(std::string_view input) {
  // Create input for PEGTL parser
  pegtl::memory_input<> in(input.data(), input.size(), "BS_");
  
  // Create state to collect results
  bit_timing_state state;
  
  try {
    // Parse input using our grammar and actions
    if (pegtl::parse<grammar::bs_rule, bit_timing_action>(in, state) &&
        state.baudrate_set && state.btr1_btr2_set) {
      // Create and return BitTiming object
      BitTiming bit_timing;
      bit_timing.baudrate = state.baudrate;
      bit_timing.btr1_btr2 = state.btr1_btr2;
      return bit_timing;
    }
  } catch (const pegtl::parse_error&) {
    // Parse error - return nullopt
    return std::nullopt;
  }
  
  return std::nullopt;
}

}  // namespace parser
}  // namespace dbc_parser 