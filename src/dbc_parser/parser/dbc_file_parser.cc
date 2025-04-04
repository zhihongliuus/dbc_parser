#include "src/dbc_parser/parser/dbc_file_parser.h"

#include <string>
#include <string_view>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/analyze.hpp"

#include "src/dbc_parser/parser/version_parser.h"
#include "src/dbc_parser/parser/new_symbols_parser.h"
#include "src/dbc_parser/parser/bit_timing_parser.h"
#include "src/dbc_parser/parser/nodes_parser.h"
#include "src/dbc_parser/parser/value_table_parser.h"
#include "src/dbc_parser/parser/message_parser.h"
#include "src/dbc_parser/parser/message_transmitters_parser.h"
#include "src/dbc_parser/parser/environment_variable_parser.h"
#include "src/dbc_parser/parser/environment_variable_data_parser.h"
// Don't include signal_parser.h directly to avoid redefinition issues
#include "src/dbc_parser/parser/signal_type_def_parser.h"
#include "src/dbc_parser/parser/signal_value_type_parser.h"
#include "src/dbc_parser/parser/signal_group_parser.h"
#include "src/dbc_parser/parser/comment_parser.h"
#include "src/dbc_parser/parser/value_description_parser.h"
#include "src/dbc_parser/parser/attribute_definition_parser.h"
#include "src/dbc_parser/parser/attribute_definition_default_parser.h"
#include "src/dbc_parser/parser/attribute_value_parser.h"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar for DBC file
namespace grammar {

// Basic whitespace and comment rules
struct space : pegtl::one<' ', '\t'> {};
struct eol : pegtl::eol {};
struct comment : pegtl::seq<pegtl::string<'/', '/'>, pegtl::until<eol>> {};
struct ws : pegtl::star<space> {};
struct blank_line : pegtl::seq<ws, eol> {};
struct ignored : pegtl::sor<comment, blank_line> {};

// Keywords for section identification
struct version_key : pegtl::string<'V', 'E', 'R', 'S', 'I', 'O', 'N'> {};
struct new_symbols_key : pegtl::string<'N', 'S', '_'> {};
struct bit_timing_key : pegtl::string<'B', 'S', '_'> {};
struct nodes_key : pegtl::string<'B', 'U', '_'> {};
struct value_table_key : pegtl::string<'V', 'A', 'L', '_', 'T', 'A', 'B', 'L', 'E', '_'> {};
struct message_key : pegtl::string<'B', 'O', '_'> {};
struct message_transmitters_key : pegtl::string<'B', 'O', '_', 'T', 'X', '_', 'B', 'U', '_'> {};

// Rules for capturing line content
struct line_content : pegtl::until<pegtl::eol> {};
struct indented_line : pegtl::seq<ws, pegtl::plus<space>, pegtl::not_at<pegtl::eol>, pegtl::until<pegtl::eol>> {};

// VERSION section
struct version_section : pegtl::seq<
                           pegtl::at<version_key>, 
                           pegtl::until<pegtl::eol>> {};

// NS_ section (with possible continuation lines)
struct new_symbols_section : pegtl::seq<
                               pegtl::at<new_symbols_key>, 
                               pegtl::until<pegtl::eol>> {};

// BU_ section
struct nodes_section : pegtl::seq<
                         pegtl::at<nodes_key>, 
                         pegtl::until<pegtl::eol>> {};

// BO_ section
struct message_section : pegtl::seq<
                           pegtl::at<message_key>,
                           pegtl::until<pegtl::eol>> {};

// BO_TX_BU_ section
struct message_transmitters_section : pegtl::seq<
                                        pegtl::at<message_transmitters_key>,
                                        pegtl::until<pegtl::eol>> {};

// Any line with content
struct any_line : pegtl::seq<pegtl::not_at<eol>, pegtl::until<eol>> {};

// Main grammar
struct dbc_file : pegtl::until<pegtl::eof, 
                    pegtl::sor<
                      version_section,
                      new_symbols_section,
                      nodes_section,
                      message_section,
                      message_transmitters_section,
                      indented_line,
                      ignored,
                      any_line>> {};

} // namespace grammar

// State for parsing
struct dbc_state {
  DbcFile dbc_file;
  std::string current_section_type;
  std::string accumulator;
  bool in_section = false;
  bool found_valid_section = false;
  
  // Start a new section
  void start_section(const std::string& section_type, const std::string& content) {
    // Process any existing section
    process_current_section();
    
    // Start the new section
    current_section_type = section_type;
    accumulator = content;
    in_section = true;
  }
  
  // Add a line to the current section
  void add_to_section(const std::string& line) {
    if (in_section) {
      accumulator += "\n" + line;
    }
  }
  
  // Process the current section and update the DbcFile
  void process_current_section() {
    if (!in_section || accumulator.empty()) {
      return;
    }
    
    if (current_section_type == "VERSION") {
      auto version_result = VersionParser::Parse(accumulator);
      if (version_result) {
        dbc_file.version = version_result->version;
        found_valid_section = true;
      }
    } else if (current_section_type == "NS_") {
      auto symbols_result = NewSymbolsParser::Parse(accumulator);
      if (symbols_result) {
        dbc_file.new_symbols = symbols_result->symbols;
        found_valid_section = true;
      }
    } else if (current_section_type == "BU_") {
      auto nodes_result = NodesParser::Parse(accumulator);
      if (nodes_result) {
        dbc_file.nodes.clear();
        for (const auto& node : nodes_result->nodes) {
          dbc_file.nodes.push_back(node.name);
        }
        found_valid_section = true;
      }
    } else if (current_section_type == "BO_") {
      auto message_result = MessageParser::Parse(accumulator);
      if (message_result) {
        dbc_file.messages[message_result->id] = message_result->name;
        found_valid_section = true;
      }
    } else if (current_section_type == "BO_TX_BU_") {
      auto transmitters_result = MessageTransmittersParser::Parse(accumulator);
      if (transmitters_result) {
        dbc_file.message_transmitters[transmitters_result->message_id] = 
            transmitters_result->transmitters;
        found_valid_section = true;
      }
    }
    
    // Reset for next section
    in_section = false;
    current_section_type.clear();
    accumulator.clear();
  }
};

// Actions for grammar rules
template <typename Rule>
struct action : pegtl::nothing<Rule> {};

// Action for VERSION section
template <>
struct action<grammar::version_section> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.start_section("VERSION", in.string());
  }
};

// Action for NS_ section
template <>
struct action<grammar::new_symbols_section> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.start_section("NS_", in.string());
  }
};

// Action for BU_ section
template <>
struct action<grammar::nodes_section> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.start_section("BU_", in.string());
  }
};

// Action for BO_ section
template <>
struct action<grammar::message_section> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.start_section("BO_", in.string());
  }
};

// Action for BO_TX_BU_ section
template <>
struct action<grammar::message_transmitters_section> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.start_section("BO_TX_BU_", in.string());
  }
};

// Action for indented (continuation) lines
template <>
struct action<grammar::indented_line> {
  template <typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.add_to_section(in.string());
  }
};

// Main parser implementation
std::optional<DbcFile> DbcFileParser::Parse(std::string_view input) {
  // Check grammar for potential issues
  if (const std::size_t issues = pegtl::analyze<grammar::dbc_file>()) {
    // We can still try to parse even if analysis shows issues
  }

  try {
    // Initialize parsing state
    dbc_state state;
    
    // Parse input using PEGTL
    pegtl::memory_input in(input.data(), input.size(), "DBC file");
    if (pegtl::parse<grammar::dbc_file, action>(in, state)) {
      // Process any final section that might be pending
      state.process_current_section();
      
      // Return result if we found at least one valid section
      if (state.found_valid_section) {
        return state.dbc_file;
      }
    }
  } catch (const pegtl::parse_error& e) {
    // Handle parsing errors
    return std::nullopt;
  }

  return std::nullopt;
}

}  // namespace parser
}  // namespace dbc_parser 