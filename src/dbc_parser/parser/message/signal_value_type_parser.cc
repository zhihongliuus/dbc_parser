#include "src/dbc_parser/parser/message/signal_value_type_parser.h"

#include <optional>
#include <string>
#include <string_view>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/parse_tree.hpp"
#include "src/dbc_parser/common/common_grammar.h"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for SIG_VALTYPE_ (Signal Value Type) parsing
namespace grammar {

// Use common grammar elements
using ws = common_grammar::ws;
using semicolon = common_grammar::semicolon;
using identifier = common_grammar::identifier;
using message_id = common_grammar::message_id;

// SIG_VALTYPE_ keyword
struct sig_valtype_keyword : pegtl::string<'S', 'I', 'G', '_', 'V', 'A', 'L', 'T', 'Y', 'P', 'E', '_'> {};

// Signal type (integer)
struct signal_type : pegtl::plus<pegtl::digit> {};

// Complete SIG_VALTYPE_ rule
struct sig_valtype_rule : pegtl::seq<
                            ws,
                            sig_valtype_keyword,
                            ws,
                            message_id,   // Message ID
                            ws,
                            identifier,   // Signal name
                            ws,
                            signal_type,  // Signal type (0, 1, or 2)
                            ws,
                            semicolon,
                            pegtl::eolf
                          > {};

} // namespace grammar

// Data structure to collect parsing results
struct signal_value_type_state {
  int message_id = 0;
  std::string signal_name;
  int type = 0;
  
  // Flags to track parsing progress
  bool message_id_set = false;
  bool signal_name_set = false;
  bool type_set = false;
};

// PEGTL actions
template<typename Rule>
struct signal_value_type_action : pegtl::nothing<Rule> {};

// Action for extracting message ID
template<>
struct signal_value_type_action<grammar::message_id> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_value_type_state& state) {
    if (!state.message_id_set) {
      state.message_id = std::stoi(in.string());
      state.message_id_set = true;
    }
  }
};

// Action for extracting signal name
template<>
struct signal_value_type_action<grammar::identifier> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_value_type_state& state) {
    if (!state.signal_name_set) {
      state.signal_name = in.string();
      state.signal_name_set = true;
    }
  }
};

// Action for extracting signal type
template<>
struct signal_value_type_action<grammar::signal_type> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, signal_value_type_state& state) {
    if (!state.type_set) {
      state.type = std::stoi(in.string());
      state.type_set = true;
    }
  }
};

std::optional<SignalValueType> SignalValueTypeParser::Parse(std::string_view input) {
  // Validate input using ParserBase method
  if (!ValidateInput(input)) {
    return std::nullopt;
  }
  
  // Create state to collect results
  signal_value_type_state state;
  
  try {
    // Create input for PEGTL parser using base class method
    pegtl::memory_input<> in = CreateInput(input, "SIG_VALTYPE_");
    
    // Parse input using our grammar and actions
    if (!pegtl::parse<grammar::sig_valtype_rule, signal_value_type_action>(in, state)) {
      return std::nullopt;
    }
    
    // Verify required fields are set
    if (!state.message_id_set || !state.signal_name_set || !state.type_set) {
      return std::nullopt;
    }
    
    // Validate signal type (must be 0, 1, or 2)
    if (state.type < 0 || state.type > 2) {
      return std::nullopt;
    }
    
    // Create and return SignalValueType object if parsing succeeded
    SignalValueType signal_value_type;
    signal_value_type.message_id = state.message_id;
    signal_value_type.signal_name = state.signal_name;
    signal_value_type.type = state.type;
    
    return signal_value_type;
  } catch (const pegtl::parse_error&) {
    // Parse error - return nullopt
    return std::nullopt;
  }
}

}  // namespace parser
}  // namespace dbc_parser 