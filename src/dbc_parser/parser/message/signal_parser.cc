#include "dbc_parser/parser/message/signal_parser.h"

#include <cstdlib>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>

#include "dbc_parser/common/common_grammar.h"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for parsing signals in DBC files.
namespace grammar {

// Use common grammar elements
using ws = common_grammar::ws;
using required_ws = common_grammar::req_ws;
using sg_prefix = common_grammar::sg_keyword;
using colon = common_grammar::colon;
using quoted_string = common_grammar::quoted_string;

// Signal name
struct signal_name : pegtl::identifier {};

// Multiplexer options
struct multiplexer : pegtl::one<'M'> {};
struct multiplexed_by : pegtl::seq<pegtl::one<'m'>, pegtl::plus<pegtl::digit>> {};
struct multiplexer_info : pegtl::sor<multiplexer, multiplexed_by, pegtl::success> {};

// Signal bit position and size
struct digits : pegtl::plus<pegtl::digit> {};
struct sign : pegtl::opt<pegtl::one<'-'>> {};
struct signed_number : pegtl::seq<sign, digits> {};
struct bit_position : pegtl::plus<pegtl::digit> {};
struct bit_size : pegtl::plus<pegtl::digit> {};
struct bit_info : pegtl::seq<bit_position, pegtl::one<'|'>, bit_size> {};

// Byte order and sign
struct byte_order : pegtl::one<'0', '1'> {};
struct signal_sign : pegtl::one<'+', '-'> {};
struct byte_order_and_sign : pegtl::seq<pegtl::one<'@'>, byte_order, signal_sign> {};

// Factor and offset
struct real_number : pegtl::seq<
    pegtl::opt<pegtl::one<'-'>>,  
    pegtl::plus<pegtl::digit>,
    pegtl::opt<pegtl::seq<
        pegtl::one<'.'>,
        pegtl::star<pegtl::digit>
    >>
> {};
struct factor : real_number {};
struct offset : real_number {};
struct factor_offset : pegtl::seq<
    pegtl::one<'('>, ws,
    factor, ws, pegtl::one<','>, ws,
    offset, ws,
    pegtl::one<')'>
> {};

// Minimum and maximum values
struct minimum : real_number {};
struct maximum : real_number {};
struct range : pegtl::seq<
    pegtl::one<'['>, ws,
    minimum, ws, pegtl::one<'|'>, ws,
    maximum, ws,
    pegtl::one<']'>
> {};

// Unit
struct unit : quoted_string {};

// Receiving nodes
struct node_name : pegtl::identifier {};
struct receiver_list : pegtl::list<node_name, pegtl::one<','>, pegtl::space> {};

// The complete signal rule
struct signal_rule : pegtl::seq<
    sg_prefix, required_ws,
    signal_name, ws,
    multiplexer_info, ws,
    colon, ws,
    bit_info, ws,
    byte_order_and_sign, ws,
    factor_offset, ws,
    range, ws,
    unit, ws,
    receiver_list, pegtl::eof
> {};

} // namespace grammar

// Class to accumulate parsed data
struct signal_state {
  std::optional<std::string> name;
  std::optional<int> start_bit;
  std::optional<int> signal_size;
  std::optional<bool> is_little_endian;
  std::optional<bool> is_signed;
  std::optional<double> factor;
  std::optional<double> offset;
  std::optional<double> minimum;
  std::optional<double> maximum;
  std::optional<std::string> unit;
  std::vector<std::string> receivers;
  bool is_multiplexer = false;
  std::optional<int> multiplex_value;

  bool is_complete() const {
    return name.has_value() && 
           start_bit.has_value() && 
           signal_size.has_value() && 
           is_little_endian.has_value() && 
           is_signed.has_value() && 
           factor.has_value() && 
           offset.has_value() && 
           minimum.has_value() && 
           maximum.has_value() && 
           unit.has_value() && 
           !receivers.empty();
  }
};

// Actions to extract data during parsing
template<typename Rule>
struct action : pegtl::nothing<Rule> {};

template<>
struct action<grammar::signal_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_state& state) {
    if (!state.name.has_value()) {
      state.name = in.string();
    }
  }
};

template<>
struct action<grammar::multiplexer> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_state& state) {
    state.is_multiplexer = true;
  }
};

template<>
struct action<grammar::multiplexed_by> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_state& state) {
    std::string value = in.string();
    // Skip the 'm' prefix
    try {
      state.multiplex_value = std::stoi(value.substr(1));
    } catch (const std::exception& e) {
      // Error in conversion, leave as nullopt
    }
  }
};

template<>
struct action<grammar::bit_position> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_state& state) {
    if (!state.start_bit.has_value()) {
      try {
        state.start_bit = std::stoi(in.string());
      } catch (const std::exception& e) {
        // Error in conversion, leave as nullopt
      }
    }
  }
};

template<>
struct action<grammar::bit_size> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_state& state) {
    if (!state.signal_size.has_value()) {
      try {
        state.signal_size = std::stoi(in.string());
      } catch (const std::exception& e) {
        // Error in conversion, leave as nullopt
      }
    }
  }
};

template<>
struct action<grammar::byte_order_and_sign> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_state& state) {
    std::string value = in.string();
    if (value.length() >= 3) {
      // Format is @XY where X is byte order (0=big endian, 1=little endian)
      // and Y is sign (+=signed, -=unsigned)
      state.is_little_endian = (value[1] == '1');
      state.is_signed = (value[2] == '+');
    }
  }
};

template<>
struct action<grammar::factor> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_state& state) {
    if (!state.factor.has_value()) {
      try {
        state.factor = std::stod(in.string());
      } catch (const std::exception& e) {
        // Error in conversion, leave as nullopt
        state.factor = 1.0;  // Default factor
      }
    }
  }
};

template<>
struct action<grammar::offset> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_state& state) {
    if (!state.offset.has_value()) {
      try {
        state.offset = std::stod(in.string());
      } catch (const std::exception& e) {
        // Error in conversion, leave as nullopt
        state.offset = 0.0;  // Default offset
      }
    }
  }
};

template<>
struct action<grammar::minimum> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_state& state) {
    if (!state.minimum.has_value()) {
      try {
        state.minimum = std::stod(in.string());
      } catch (const std::exception& e) {
        // Error in conversion, leave as nullopt
        state.minimum = 0.0;  // Default minimum
      }
    }
  }
};

template<>
struct action<grammar::maximum> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_state& state) {
    if (!state.maximum.has_value()) {
      try {
        state.maximum = std::stod(in.string());
      } catch (const std::exception& e) {
        // Error in conversion, leave as nullopt
        state.maximum = 0.0;  // Default maximum
      }
    }
  }
};

template<>
struct action<grammar::unit> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_state& state) {
    if (!state.unit.has_value()) {
      std::string unit = in.string();
      // Remove quotes
      if (unit.length() >= 2) {
        state.unit = unit.substr(1, unit.length() - 2);
      } else {
        state.unit = "";
      }
    }
  }
};

template<>
struct action<grammar::node_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_state& state) {
    state.receivers.push_back(in.string());
  }
};

std::optional<Signal> SignalParser::Parse(std::string_view input) {
  // Validate input using ParserBase method, but only check for empty
  if (!ValidateInput(input)) {
    return std::nullopt;
  }
  
  signal_state state;
  
  // Create input for PEGTL parser
  pegtl::memory_input<> in(input.data(), input.size(), "SG_");
  
  try {
    pegtl::parse<grammar::signal_rule, action>(in, state);
  } catch (const pegtl::parse_error& e) {
    return std::nullopt;
  }

  if (!state.is_complete()) {
    return std::nullopt;
  }

  Signal result;
  result.name = *state.name;
  result.start_bit = *state.start_bit;
  result.signal_size = *state.signal_size;
  result.length = *state.signal_size;  // Set both length and signal_size
  result.is_little_endian = *state.is_little_endian;
  result.byte_order = *state.is_little_endian ? 1 : 0;  // Set byte_order based on little endian flag
  result.is_signed = *state.is_signed;
  result.sign = *state.is_signed ? SignType::kSigned : SignType::kUnsigned;  // Set the enum value
  result.factor = *state.factor;
  result.offset = *state.offset;
  result.minimum = *state.minimum;
  result.maximum = *state.maximum;
  result.unit = *state.unit;
  result.receivers = std::move(state.receivers);
  result.is_multiplexer = state.is_multiplexer;
  
  // Update the multiplex value fields
  if (state.is_multiplexer) {
    result.multiplex_type = MultiplexType::kMultiplexor;
  } else if (state.multiplex_value.has_value()) {
    result.multiplex_type = MultiplexType::kMultiplexed;
    result.multiplex_value = state.multiplex_value;
    result.multiplex_value_int = *state.multiplex_value;
  } else {
    result.multiplex_type = MultiplexType::kNone;
  }

  return result;
}

} // namespace parser
} // namespace dbc_parser 