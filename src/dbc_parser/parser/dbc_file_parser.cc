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

// Version-specific rules
struct quoted_string : pegtl::seq<pegtl::one<'"'>, pegtl::until<pegtl::one<'"'>>, pegtl::opt<ws>> {};
struct version_content : pegtl::seq<version_key, ws, quoted_string, pegtl::until<pegtl::eol>> {};
struct invalid_version_content : pegtl::seq<version_key, ws, pegtl::not_at<pegtl::one<'"'>>, line_content> {};

// Section rules with content capturing
struct version_section : pegtl::sor<version_content, invalid_version_content> {};
struct new_symbols_line : pegtl::seq<new_symbols_key, line_content> {};
struct nodes_line : pegtl::seq<nodes_key, line_content> {};
struct message_line : pegtl::seq<message_key, line_content> {};
struct message_transmitters_line : pegtl::seq<message_transmitters_key, line_content> {};

// Complete section definitions
struct new_symbols_section : pegtl::seq<
                               new_symbols_line,
                               pegtl::star<indented_line>> {};

struct nodes_section : pegtl::seq<
                         nodes_line,
                         pegtl::star<indented_line>> {};

struct message_section : pegtl::seq<
                           message_line,
                           pegtl::star<indented_line>> {};

struct message_transmitters_section : pegtl::seq<
                                        message_transmitters_line,
                                        pegtl::star<indented_line>> {};

// Any line with content (for skipping unknown lines)
struct any_line : pegtl::seq<pegtl::not_at<eol>, pegtl::until<eol>> {};

// Main grammar rule
struct dbc_file : pegtl::until<pegtl::eof, 
                    pegtl::sor<
                      version_section,
                      new_symbols_section,
                      nodes_section,
                      message_section,
                      message_transmitters_section,
                      ignored,
                      any_line>> {};

} // namespace grammar

// State for parsing
struct dbc_state {
  DbcFile dbc_file;
  bool found_valid_section = false;
  
  // Track version validity for invalid version format test
  bool invalid_version_format = false;
  
  // Buffers for accumulating section content
  std::string version_content;
  std::string new_symbols_content;
  std::string nodes_content;
  std::string message_content;
  std::string message_transmitters_content;
  
  // Methods to set section content
  void set_version_content(const std::string& line) {
    version_content = line;
  }
  
  void set_new_symbols_content(const std::string& line) {
    new_symbols_content = line;
  }
  
  void set_nodes_content(const std::string& line) {
    nodes_content = line;
  }
  
  void set_message_content(const std::string& line) {
    message_content = line;
  }
  
  void set_message_transmitters_content(const std::string& line) {
    message_transmitters_content = line;
  }
  
  // Methods to add continuations to specific sections
  void add_to_new_symbols(const std::string& line) {
    if (!new_symbols_content.empty()) {
      new_symbols_content += "\n";
    }
    new_symbols_content += line;
  }
  
  void add_to_nodes(const std::string& line) {
    if (!nodes_content.empty()) {
      nodes_content += "\n";
    }
    nodes_content += line;
  }
  
  void add_to_message(const std::string& line) {
    if (!message_content.empty()) {
      message_content += "\n";
    }
    message_content += line;
  }
  
  void add_to_message_transmitters(const std::string& line) {
    if (!message_transmitters_content.empty()) {
      message_transmitters_content += "\n";
    }
    message_transmitters_content += line;
  }
};

// Actions for grammar rules
template <typename Rule>
struct action : pegtl::nothing<Rule> {};

// Version actions
template<>
struct action<grammar::version_content> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.set_version_content(in.string());
    
    // Process immediately to capture the version
    auto version_result = VersionParser::Parse(state.version_content);
    if (version_result) {
      state.dbc_file.version = version_result->version;
      state.found_valid_section = true;
    }
  }
};

template<>
struct action<grammar::invalid_version_content> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.invalid_version_format = true;
  }
};

// Actions for NS_ section
template<>
struct action<grammar::new_symbols_line> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.set_new_symbols_content(in.string());
  }
};

template<>
struct action<grammar::new_symbols_section> {
  template<typename ActionInput>
  static void apply(const ActionInput&, dbc_state& state) {
    if (!state.new_symbols_content.empty()) {
      auto symbols_result = NewSymbolsParser::Parse(state.new_symbols_content);
      if (symbols_result) {
        state.dbc_file.new_symbols = symbols_result->symbols;
        state.found_valid_section = true;
      }
    }
  }
};

// Actions for BU_ section
template<>
struct action<grammar::nodes_line> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.set_nodes_content(in.string());
  }
};

template<>
struct action<grammar::nodes_section> {
  template<typename ActionInput>
  static void apply(const ActionInput&, dbc_state& state) {
    if (!state.nodes_content.empty()) {
      auto nodes_result = NodesParser::Parse(state.nodes_content);
      if (nodes_result) {
        state.dbc_file.nodes.clear();
        for (const auto& node : nodes_result->nodes) {
          state.dbc_file.nodes.push_back(node.name);
        }
        state.found_valid_section = true;
      }
    }
  }
};

// Actions for BO_ section
template<>
struct action<grammar::message_line> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.set_message_content(in.string());
  }
};

template<>
struct action<grammar::message_section> {
  template<typename ActionInput>
  static void apply(const ActionInput&, dbc_state& state) {
    if (!state.message_content.empty()) {
      auto message_result = MessageParser::Parse(state.message_content);
      if (message_result) {
        state.dbc_file.messages[message_result->id] = message_result->name;
        state.found_valid_section = true;
      }
    }
  }
};

// Actions for BO_TX_BU_ section
template<>
struct action<grammar::message_transmitters_line> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.set_message_transmitters_content(in.string());
  }
};

template<>
struct action<grammar::message_transmitters_section> {
  template<typename ActionInput>
  static void apply(const ActionInput&, dbc_state& state) {
    if (!state.message_transmitters_content.empty()) {
      auto transmitters_result = MessageTransmittersParser::Parse(state.message_transmitters_content);
      if (transmitters_result) {
        state.dbc_file.message_transmitters[transmitters_result->message_id] = 
            transmitters_result->transmitters;
        state.found_valid_section = true;
      }
    }
  }
};

// Continuation line handling for each active section
template<>
struct action<grammar::indented_line> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    // Add continuation to the most recently used section
    if (!state.new_symbols_content.empty()) {
      state.add_to_new_symbols(in.string());
    } else if (!state.nodes_content.empty()) {
      state.add_to_nodes(in.string());
    } else if (!state.message_content.empty()) {
      state.add_to_message(in.string());
    } else if (!state.message_transmitters_content.empty()) {
      state.add_to_message_transmitters(in.string());
    }
    // VERSION doesn't support continuation lines
  }
};

// Main parser implementation
std::optional<DbcFile> DbcFileParser::Parse(std::string_view input) {
  // Analyze grammar for potential issues (optional)
  if (const std::size_t issues = pegtl::analyze<grammar::dbc_file>()) {
    // We can still try to parse even if analysis shows issues
  }

  try {
    // Initialize parsing state
    dbc_state state;
    
    // Parse input using PEGTL
    pegtl::memory_input in(input.data(), input.size(), "DBC file");
    if (pegtl::parse<grammar::dbc_file, action>(in, state)) {
      // Handle invalid version format test
      if (state.invalid_version_format) {
        return std::nullopt;
      }
      
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