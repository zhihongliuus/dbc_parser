#include "src/dbc_parser/parser/message/message_parser.h"

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/analyze.hpp"

#include "src/dbc_parser/common/common_grammar.h"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for BO_ (Message) and SG_ (Signal) parsing
namespace grammar {

// Use common grammar elements
using ws = common_grammar::ws;
using quoted_string = common_grammar::quoted_string;
using integer = common_grammar::integer;
using colon = common_grammar::colon;
using bo_keyword = common_grammar::bo_keyword;
using sg_keyword = common_grammar::sg_keyword;

// Float
struct floating_point : pegtl::seq<
                          pegtl::opt<pegtl::one<'-'>>,
                          pegtl::plus<pegtl::digit>,
                          pegtl::opt<
                            pegtl::seq<
                              pegtl::one<'.'>,
                              pegtl::star<pegtl::digit>
                            >
                          >
                        > {};

// Valid identifier character
struct id_char : pegtl::sor<
                   pegtl::alpha,
                   pegtl::digit,
                   pegtl::one<'_'>,
                   pegtl::one<'-'>
                 > {};

// Node name (sender or receiver)
struct node_name : pegtl::plus<id_char> {};

// Message name
struct message_name : pegtl::plus<id_char> {};

// Signal name
struct signal_name : pegtl::plus<id_char> {};

// Message ID, name, DLC, and sender
struct message_header : pegtl::seq<
                          ws,
                          bo_keyword,
                          ws,
                          integer,          // Message ID
                          ws,
                          message_name,     // Message name
                          ws,
                          colon,
                          ws,
                          integer,          // DLC (data length)
                          ws,
                          node_name         // Sender node
                        > {};

// Multiplexor indicator 'M' or multiplexed indicator 'mX'
struct multiplexor : pegtl::string<'M'> {};

struct multiplexed : pegtl::seq<
                       pegtl::string<'m'>,
                       pegtl::plus<pegtl::digit>
                     > {};

struct multiplex_indicator : pegtl::sor<
                               multiplexor,
                               multiplexed
                             > {};

// Signal start bit and length: startbit|length
struct bit_position : pegtl::seq<
                        integer,             // Start bit
                        pegtl::one<'|'>,
                        integer              // Length
                      > {};

// Signal byte order and sign: @1+ or @0-
struct format : pegtl::seq<
                  pegtl::one<'@'>,
                  pegtl::one<'0', '1'>,      // Byte order (0=Motorola, 1=Intel)
                  pegtl::one<'+', '-'>       // Sign (+=unsigned, -=signed)
                > {};

// Signal factor and offset: (factor,offset)
struct factor_offset : pegtl::seq<
                         pegtl::one<'('>,
                         floating_point,     // Factor
                         pegtl::one<','>,
                         floating_point,     // Offset
                         pegtl::one<')'>
                       > {};

// Signal min and max: [min|max]
struct min_max : pegtl::seq<
                   pegtl::one<'['>,
                   floating_point,           // Min
                   pegtl::one<'|'>,
                   floating_point,           // Max
                   pegtl::one<']'>
                 > {};

// Receiver nodes (comma-separated list)
struct receiver_list : pegtl::list<
                         node_name,
                         pegtl::one<','>,
                         pegtl::space
                       > {};

// Signal definition
struct signal_def : pegtl::seq<
                      ws,
                      sg_keyword,
                      ws,
                      signal_name,
                      ws,
                      pegtl::opt<
                        pegtl::seq<
                          multiplex_indicator,
                          ws
                        >
                      >,
                      colon,
                      ws,
                      bit_position,
                      ws,
                      format,
                      ws,
                      factor_offset,
                      ws,
                      min_max,
                      ws,
                      quoted_string,
                      ws,
                      receiver_list
                    > {};

// Multiple signal definitions
struct signal_list : pegtl::star<signal_def> {};

// Complete message rule
struct bo_rule : pegtl::seq<
                   message_header,
                   signal_list,
                   pegtl::opt<ws>,
                   pegtl::eof
                 > {};

} // namespace grammar

// Data structure to collect parsing results
struct message_state {
  Message message;
  Signal current_signal;
  bool in_signal = false;
  std::string current_multiplex;
};

// PEGTL actions
template<typename Rule>
struct action : pegtl::nothing<Rule> {};

// Message ID
template<>
struct action<grammar::integer> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, message_state& state) noexcept {
    // Determine where to store the integer value based on context
    try {
      if (!state.in_signal) {
        // We're in the message header, potential targets: ID, DLC
        if (state.message.id == 0) {
          state.message.id = std::stoi(in.string());
        } else if (state.message.dlc == 0) {
          state.message.dlc = std::stoi(in.string());
        }
      } else {
        // We're in a signal definition, potential targets: start_bit, length, etc.
        static int signal_int_field = 0;
        
        if (signal_int_field == 0) { // Start bit
          state.current_signal.start_bit = std::stoi(in.string());
          signal_int_field = 1;
        } else if (signal_int_field == 1) { // Length
          state.current_signal.length = std::stoi(in.string());
          signal_int_field = 0;
        }
      }
    } catch (const std::exception&) {
      // Handle conversion errors gracefully
    }
  }
};

// Message name
template<>
struct action<grammar::message_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, message_state& state) noexcept {
    state.message.name = in.string();
  }
};

// Sender node
template<>
struct action<grammar::node_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, message_state& state) noexcept {
    if (!state.in_signal) {
      // We're in the message header, this is the sender
      if (state.message.sender.empty()) {
        state.message.sender = in.string();
      }
    } else {
      // We're in a signal definition, this is a receiver
      state.current_signal.receivers.push_back(in.string());
    }
  }
};

// Signal name
template<>
struct action<grammar::signal_name> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, message_state& state) noexcept {
    state.in_signal = true;
    state.current_signal = Signal();
    state.current_signal.name = in.string();
  }
};

// Multiplexor indicator
template<>
struct action<grammar::multiplexor> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, message_state& state) noexcept {
    state.current_signal.multiplex_type = MultiplexType::kMultiplexor;
    state.current_multiplex = in.string();
  }
};

// Multiplexed indicator
template<>
struct action<grammar::multiplexed> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, message_state& state) noexcept {
    state.current_signal.multiplex_type = MultiplexType::kMultiplexed;
    try {
      std::string value = in.string().substr(1); // Skip 'm'
      state.current_signal.multiplex_value = std::stoi(value);
    } catch (const std::exception&) {
      // Handle conversion error
      state.current_signal.multiplex_value = 0;
    }
    state.current_multiplex = in.string();
  }
};

// Format (byte order and sign)
template<>
struct action<grammar::format> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, message_state& state) noexcept {
    std::string format = in.string();
    
    // Byte order (0=Motorola/big-endian, 1=Intel/little-endian)
    if (format.length() > 1) {
      state.current_signal.byte_order = format[1] - '0';
    }
    
    // Sign (+=unsigned, -=signed)
    if (format.length() > 2) {
      state.current_signal.sign = (format[2] == '-') ? 
                                  SignType::kSigned : 
                                  SignType::kUnsigned;
    }
  }
};

// Factor
template<>
struct action<grammar::factor_offset> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, message_state& state) noexcept {
    try {
      std::string str = in.string();
      size_t comma_pos = str.find(',');
      if (comma_pos != std::string::npos) {
        // Extract factor and offset, stripping parentheses
        std::string factor_str = str.substr(1, comma_pos - 1);
        std::string offset_str = str.substr(comma_pos + 1, str.length() - comma_pos - 2);
        
        state.current_signal.factor = std::stod(factor_str);
        state.current_signal.offset = std::stod(offset_str);
      }
    } catch (const std::exception&) {
      // Handle conversion error
      state.current_signal.factor = 1.0;
      state.current_signal.offset = 0.0;
    }
  }
};

// Min/Max
template<>
struct action<grammar::min_max> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, message_state& state) noexcept {
    try {
      std::string str = in.string();
      size_t pipe_pos = str.find('|');
      if (pipe_pos != std::string::npos) {
        // Extract min and max, stripping brackets
        std::string min_str = str.substr(1, pipe_pos - 1);
        std::string max_str = str.substr(pipe_pos + 1, str.length() - pipe_pos - 2);
        
        state.current_signal.minimum = std::stod(min_str);
        state.current_signal.maximum = std::stod(max_str);
      }
    } catch (const std::exception&) {
      // Handle conversion error
      state.current_signal.minimum = 0.0;
      state.current_signal.maximum = 0.0;
    }
  }
};

// Unit
template<>
struct action<common_grammar::quoted_string> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, message_state& state) noexcept {
    // Use ParserBase method to unescape string
    state.current_signal.unit = ParserBase::UnescapeString(in.string());
  }
};

// Complete signal definition
template<>
struct action<grammar::signal_def> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, message_state& state) noexcept {
    // Add the completed signal to the message
    state.message.signals.push_back(state.current_signal);
    
    // Reset signal state
    state.in_signal = false;
    state.current_signal = Signal();
    state.current_multiplex.clear();
  }
};

std::optional<Message> MessageParser::Parse(std::string_view input) {
  // Validate input using base class method
  if (!ValidateInput(input)) {
    return std::nullopt;
  }

  // Create input for PEGTL parser using base class method
  pegtl::memory_input<> in = CreateInput(input, "BO_");
  
  // Create state to collect results
  message_state state;
  
  try {
    // Parse input using our grammar and actions
    if (pegtl::parse<grammar::bo_rule, action>(in, state)) {
      return state.message;
    }
  } catch (const pegtl::parse_error&) {
    // Parse error - return nullopt
    return std::nullopt;
  }
  
  return std::nullopt;
}

}  // namespace parser
}  // namespace dbc_parser 