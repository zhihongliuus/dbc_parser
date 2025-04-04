#include "src/dbc_parser/parser/dbc_file_parser.h"

#include <iostream>
#include <string>
#include <string_view>
#include <memory>
#include <optional>
#include <map>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>

#include "src/dbc_parser/parser/version_parser.h"
#include "src/dbc_parser/parser/new_symbols_parser.h"
#include "src/dbc_parser/parser/nodes_parser.h"
#include "src/dbc_parser/parser/message_parser.h"
#include "src/dbc_parser/parser/message_transmitters_parser.h"
#include "src/dbc_parser/parser/bit_timing_parser.h"
#include "src/dbc_parser/parser/value_table_parser.h"
#include "src/dbc_parser/parser/environment_variable_parser.h"
#include "src/dbc_parser/parser/environment_variable_data_parser.h"

// Instead of including signal_parser.h directly which causes a conflict,
// forward declare the SignalParser class
namespace dbc_parser {
namespace parser {
class SignalParser;
}  // namespace parser
}  // namespace dbc_parser

// Only include parsers that are actually used in this file
// Other parsers are included in the BUILD file for future implementation

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
struct nodes_key : pegtl::string<'B', 'U', '_'> {};
struct message_key : pegtl::string<'B', 'O', '_'> {};
struct message_transmitters_key : pegtl::string<'B', 'O', '_', 'T', 'X', '_', 'B', 'U', '_'> {};
struct bit_timing_key : pegtl::string<'B', 'S', '_'> {};
struct value_table_key : pegtl::string<'V', 'A', 'L', '_', 'T', 'A', 'B', 'L', 'E', '_'> {};
struct signal_key : pegtl::string<'S', 'G', '_'> {};
struct env_var_key : pegtl::string<'E', 'V', '_'> {};
struct env_var_data_key : pegtl::string<'E', 'N', 'V', 'V', 'A', 'R', '_', 'D', 'A', 'T', 'A', '_'> {};
struct comment_key : pegtl::string<'C', 'M', '_'> {};
struct attr_def_key : pegtl::string<'B', 'A', '_', 'D', 'E', 'F', '_'> {};
struct attr_def_def_key : pegtl::string<'B', 'A', '_', 'D', 'E', 'F', '_', 'D', 'E', 'F', '_'> {};
struct attr_key : pegtl::string<'B', 'A', '_'> {};
struct value_desc_key : pegtl::string<'V', 'A', 'L', '_'> {};
struct sig_val_type_key : pegtl::string<'S', 'I', 'G', '_', 'V', 'A', 'L', 'T', 'Y', 'P', 'E', '_'> {};
struct sig_group_key : pegtl::string<'S', 'I', 'G', '_', 'G', 'R', 'O', 'U', 'P', '_'> {};
struct sig_mul_val_key : pegtl::string<'S', 'G', '_', 'M', 'U', 'L', '_', 'V', 'A', 'L', '_'> {};

// Rules for capturing line content
struct line_content : pegtl::until<pegtl::eol> {};
struct indented_line : pegtl::seq<pegtl::opt<ws>, pegtl::plus<space>, pegtl::not_at<pegtl::eol>, pegtl::until<pegtl::eol>> {};

// Version-specific rules
struct quoted_string : pegtl::seq<pegtl::one<'"'>, pegtl::until<pegtl::one<'"'>>, pegtl::opt<ws>> {};
struct version_content : pegtl::seq<version_key, ws, quoted_string, pegtl::until<pegtl::eol>> {};
struct invalid_version_content : pegtl::seq<version_key, ws, pegtl::not_at<pegtl::one<'"'>>, line_content> {};

// Section rules with content capturing
struct version_section : pegtl::sor<version_content, invalid_version_content> {};
struct new_symbols_line : pegtl::seq<new_symbols_key, line_content> {};
struct nodes_line : pegtl::seq<nodes_key, line_content> {};
struct message_line : pegtl::seq<message_key, line_content> {};

// Special handling for message transmitters with semicolon
struct message_transmitters_line : pegtl::seq<message_transmitters_key, pegtl::until<pegtl::eol>> {};

struct bit_timing_line : pegtl::seq<bit_timing_key, line_content> {};
struct value_table_line : pegtl::seq<value_table_key, line_content> {};
struct env_var_line : pegtl::seq<env_var_key, line_content> {};
struct env_var_data_line : pegtl::seq<env_var_data_key, line_content> {};
struct comment_line : pegtl::seq<comment_key, line_content> {};
struct attr_def_line : pegtl::seq<attr_def_key, line_content> {};
struct attr_def_def_line : pegtl::seq<attr_def_def_key, line_content> {};
struct attr_line : pegtl::seq<attr_key, line_content> {};
struct value_desc_line : pegtl::seq<value_desc_key, line_content> {};
struct sig_val_type_line : pegtl::seq<sig_val_type_key, line_content> {};
struct sig_group_line : pegtl::seq<sig_group_key, line_content> {};
struct sig_mul_val_line : pegtl::seq<sig_mul_val_key, line_content> {};

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

// Special handling for message transmitters section that may include a semicolon
struct message_transmitters_section : message_transmitters_line {};

struct bit_timing_section : pegtl::seq<
                              bit_timing_line,
                              pegtl::star<indented_line>> {};

struct value_table_section : pegtl::seq<
                               value_table_line,
                               pegtl::star<indented_line>> {};

struct env_var_section : pegtl::seq<
                           env_var_line,
                           pegtl::star<indented_line>> {};

struct env_var_data_section : pegtl::seq<
                                env_var_data_line,
                                pegtl::star<indented_line>> {};

struct comment_section : pegtl::seq<
                           comment_line,
                           pegtl::star<indented_line>> {};

struct attr_def_section : pegtl::seq<
                            attr_def_line,
                            pegtl::star<indented_line>> {};

struct attr_def_def_section : pegtl::seq<
                                attr_def_def_line,
                                pegtl::star<indented_line>> {};

struct attr_section : pegtl::seq<
                        attr_line,
                        pegtl::star<indented_line>> {};

struct value_desc_section : pegtl::seq<
                              value_desc_line,
                              pegtl::star<indented_line>> {};

struct sig_val_type_section : pegtl::seq<
                                sig_val_type_line,
                                pegtl::star<indented_line>> {};

struct sig_group_section : pegtl::seq<
                             sig_group_line,
                             pegtl::star<indented_line>> {};

struct sig_mul_val_section : pegtl::seq<
                               sig_mul_val_line,
                               pegtl::star<indented_line>> {};

// Any line with content (for skipping unknown lines)
struct any_line : pegtl::seq<pegtl::not_at<eol>, pegtl::until<eol>> {};

// Main grammar rule
struct dbc_file : pegtl::until<pegtl::eof, 
                    pegtl::sor<
                      version_section,
                      new_symbols_section,
                      bit_timing_section,
                      nodes_section,
                      value_table_section,
                      message_section,
                      signal_key,
                      message_transmitters_section,
                      env_var_section,
                      env_var_data_section,
                      comment_section,
                      attr_def_section,
                      attr_def_def_section,
                      attr_section,
                      value_desc_section,
                      sig_val_type_section,
                      sig_group_section,
                      sig_mul_val_section,
                      ignored,
                      any_line>> {};

} // namespace grammar

// State for parsing
struct dbc_state {
  DbcFile dbc_file;
  bool found_valid_section = false;
  
  // Track version validity for invalid version format test
  bool invalid_version_format = false;
  
  // Track which section was last seen to properly handle indented lines
  enum SectionType {
    None,
    Version,
    NewSymbols,
    BitTiming,
    Nodes,
    Message,
    MessageTransmitters,
    ValueTable,
    EnvVar,
    EnvVarData,
    Comment,
    AttrDef,
    AttrDefDef,
    Attr,
    ValueDesc,
    SigValType,
    SigGroup,
    SigMulVal
  };
  
  SectionType current_section = SectionType::None;
  
  // Track current message ID for signal association
  int current_message_id = -1;
  
  // Buffers for accumulating section content
  std::string version_content;
  std::string new_symbols_content;
  std::string nodes_content;
  std::string message_content;
  std::string message_transmitters_content;
  std::string bit_timing_content;
  std::string value_table_content;
  std::string env_var_content;
  std::string env_var_data_content;
  std::string comment_content;
  std::string attr_def_content;
  std::string attr_def_def_content;
  std::string attr_content;
  std::string value_desc_content;
  std::string sig_val_type_content;
  std::string sig_group_content;
  std::string sig_mul_val_content;
  
  // Methods to set section content
  void set_version_content(const std::string& line) {
    version_content = line;
    current_section = SectionType::Version;
  }
  
  void set_new_symbols_content(const std::string& line) {
    new_symbols_content = line;
    current_section = SectionType::NewSymbols;
  }
  
  void set_nodes_content(const std::string& line) {
    nodes_content = line;
    current_section = SectionType::Nodes;
  }
  
  void set_message_content(const std::string& line) {
    message_content = line;
    current_section = SectionType::Message;
  }
  
  void set_message_transmitters_content(const std::string& line) {
    // Extract only the part after BO_TX_BU_ and before the semicolon
    std::string content = line;
    size_t prefix_pos = content.find("BO_TX_BU_");
    if (prefix_pos != std::string::npos) {
      content = content.substr(prefix_pos + 9); // Length of "BO_TX_BU_"
    }
    
    // Remove the trailing semicolon if present
    if (!content.empty() && content.back() == ';') {
      content.pop_back();
    }
    
    message_transmitters_content = content;
    current_section = SectionType::MessageTransmitters;
  }
  
  void set_bit_timing_content(const std::string& line) {
    bit_timing_content = line;
    current_section = SectionType::BitTiming;
  }
  
  void set_value_table_content(const std::string& line) {
    value_table_content = line;
    current_section = SectionType::ValueTable;
  }
  
  void set_env_var_content(const std::string& line) {
    env_var_content = line;
    current_section = SectionType::EnvVar;
  }
  
  void set_env_var_data_content(const std::string& line) {
    env_var_data_content = line;
    current_section = SectionType::EnvVarData;
  }
  
  void set_comment_content(const std::string& line) {
    comment_content = line;
    current_section = SectionType::Comment;
  }
  
  void set_attr_def_content(const std::string& line) {
    attr_def_content = line;
    current_section = SectionType::AttrDef;
  }
  
  void set_attr_def_def_content(const std::string& line) {
    attr_def_def_content = line;
    current_section = SectionType::AttrDefDef;
  }
  
  void set_attr_content(const std::string& line) {
    attr_content = line;
    current_section = SectionType::Attr;
  }
  
  void set_value_desc_content(const std::string& line) {
    value_desc_content = line;
    current_section = SectionType::ValueDesc;
  }
  
  void set_sig_val_type_content(const std::string& line) {
    sig_val_type_content = line;
    current_section = SectionType::SigValType;
  }
  
  void set_sig_group_content(const std::string& line) {
    sig_group_content = line;
    current_section = SectionType::SigGroup;
  }
  
  void set_sig_mul_val_content(const std::string& line) {
    sig_mul_val_content = line;
    current_section = SectionType::SigMulVal;
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
  
  void add_to_bit_timing(const std::string& line) {
    if (!bit_timing_content.empty()) {
      bit_timing_content += "\n";
    }
    bit_timing_content += line;
  }
  
  void add_to_value_table(const std::string& line) {
    if (!value_table_content.empty()) {
      value_table_content += "\n";
    }
    value_table_content += line;
  }
  
  void add_to_env_var(const std::string& line) {
    if (!env_var_content.empty()) {
      env_var_content += "\n";
    }
    env_var_content += line;
  }
  
  void add_to_env_var_data(const std::string& line) {
    if (!env_var_data_content.empty()) {
      env_var_data_content += "\n";
    }
    env_var_data_content += line;
  }
  
  void add_to_comment(const std::string& line) {
    if (!comment_content.empty()) {
      comment_content += "\n";
    }
    comment_content += line;
  }
  
  void add_to_attr_def(const std::string& line) {
    if (!attr_def_content.empty()) {
      attr_def_content += "\n";
    }
    attr_def_content += line;
  }
  
  void add_to_attr_def_def(const std::string& line) {
    if (!attr_def_def_content.empty()) {
      attr_def_def_content += "\n";
    }
    attr_def_def_content += line;
  }
  
  void add_to_attr(const std::string& line) {
    if (!attr_content.empty()) {
      attr_content += "\n";
    }
    attr_content += line;
  }
  
  void add_to_value_desc(const std::string& line) {
    if (!value_desc_content.empty()) {
      value_desc_content += "\n";
    }
    value_desc_content += line;
  }
  
  void add_to_sig_val_type(const std::string& line) {
    if (!sig_val_type_content.empty()) {
      sig_val_type_content += "\n";
    }
    sig_val_type_content += line;
  }
  
  void add_to_sig_group(const std::string& line) {
    if (!sig_group_content.empty()) {
      sig_group_content += "\n";
    }
    sig_group_content += line;
  }
  
  void add_to_sig_mul_val(const std::string& line) {
    if (!sig_mul_val_content.empty()) {
      sig_mul_val_content += "\n";
    }
    sig_mul_val_content += line;
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
        // Store basic message info in the 'messages' map
        state.dbc_file.messages[message_result->id] = message_result->name;
        // Update current message ID for signal association
        state.current_message_id = message_result->id;
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

// Complete parsing of the message transmitters section
template<>
struct action<grammar::message_transmitters_section> {
  template<typename ActionInput>
  static void apply(const ActionInput&, dbc_state& state) {
    if (!state.message_transmitters_content.empty()) {
      std::string full_content = "BO_TX_BU_ " + state.message_transmitters_content + ";";
      auto transmitters_result = MessageTransmittersParser::Parse(full_content);
      if (transmitters_result) {
        state.dbc_file.message_transmitters[transmitters_result->message_id] = 
            transmitters_result->transmitters;
        state.found_valid_section = true;
      }
    }
  }
};

// Actions for BS_ section
template<>
struct action<grammar::bit_timing_line> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.set_bit_timing_content(in.string());
  }
};

template<>
struct action<grammar::bit_timing_section> {
  template<typename ActionInput>
  static void apply(const ActionInput&, dbc_state& state) {
    if (!state.bit_timing_content.empty()) {
      // Just mark that we found a valid section
      // In a future implementation, we would parse the bit timing content
      state.found_valid_section = true;
      
      // For a proper implementation, we would do something like:
      // auto bit_timing_result = BitTimingParser::Parse(state.bit_timing_content);
      // if (bit_timing_result) {
      //   // Set the result in the DbcFile
      // }
    }
  }
};

// Actions for VAL_TABLE_ section
template<>
struct action<grammar::value_table_line> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.set_value_table_content(in.string());
  }
};

template<>
struct action<grammar::value_table_section> {
  template<typename ActionInput>
  static void apply(const ActionInput&, dbc_state& state) {
    if (!state.value_table_content.empty()) {
      auto value_table_result = ValueTableParser::Parse(state.value_table_content);
      if (value_table_result) {
        // Convert from unordered_map to map
        std::map<int, std::string> values_map;
        for (const auto& [key, value] : value_table_result->values) {
          values_map[key] = value;
        }
        
        // Add the value table to the map
        state.dbc_file.value_tables[value_table_result->name] = values_map;
        state.found_valid_section = true;
      }
    }
  }
};

// Actions for SG_ section - implemented to handle signals within messages
// For now, we just detect SG_ lines for validation, but don't try to parse them
// due to the Signal struct conflict
template<>
struct action<grammar::signal_key> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    // Only process signals if we have a current message ID
    if (state.current_message_id >= 0) {
      // Just mark that we found a signal section
      // This is a placeholder for future signal parsing implementation
      state.found_valid_section = true;
      
      // TODO: Future implementation will:
      // 1. Define a common Signal representation or create an adapter
      // 2. Include signal_parser.h in a way that doesn't conflict
      // 3. Parse signals and associate them with their messages
    }
  }
};

// Actions for EV_ section
template<>
struct action<grammar::env_var_line> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.set_env_var_content(in.string());
  }
};

template<>
struct action<grammar::env_var_section> {
  template<typename ActionInput>
  static void apply(const ActionInput&, dbc_state& state) {
    if (!state.env_var_content.empty()) {
      std::cout << "Original environment variable content: '" << state.env_var_content << "'" << std::endl;
      
      // Parse environment variable directly without using EnvironmentVariableParser
      std::string content = state.env_var_content;
      // Remove trailing newlines
      while (!content.empty() && (content.back() == '\n' || content.back() == '\r')) {
        content.pop_back();
      }
      
      // Expected format: EV_ EngineTemp 1 [0|120] "C" 20 0 DUMMY_NODE_VECTOR0 Vector__XXX;
      // Split by spaces but handle quoted strings and bracket content
      std::vector<std::string> tokens;
      bool in_quotes = false;
      bool in_brackets = false;
      std::string current_token;
      
      for (size_t i = 0; i < content.size(); ++i) {
        char c = content[i];
        
        if (c == '"') {
          in_quotes = !in_quotes;
          current_token += c;
        } else if (c == '[') {
          in_brackets = true;
          current_token += c;
        } else if (c == ']') {
          in_brackets = false;
          current_token += c;
        } else if ((c == ' ' || c == '\t') && !in_quotes && !in_brackets) {
          if (!current_token.empty()) {
            tokens.push_back(current_token);
            current_token.clear();
          }
        } else if (c == ';' && !in_quotes && !in_brackets) {
          // End of the statement
          if (!current_token.empty()) {
            tokens.push_back(current_token);
            current_token.clear();
          }
        } else {
          current_token += c;
        }
      }
      
      // Add the last token if not empty
      if (!current_token.empty()) {
        tokens.push_back(current_token);
      }
      
      // Debug output to see tokens
      std::cout << "Parsed " << tokens.size() << " tokens:" << std::endl;
      for (size_t i = 0; i < tokens.size(); ++i) {
        std::cout << "  " << i << ": '" << tokens[i] << "'" << std::endl;
      }
      
      // Extract values from tokens
      if (tokens.size() >= 8 && tokens[0] == "EV_") {
        DbcFile::EnvVar env_var;
        
        // Name is the first token after EV_
        env_var.name = tokens[1];
        
        // Type is the second token after EV_
        env_var.type = std::stoi(tokens[2]);
        
        // Parse range [min|max]
        std::string range = tokens[3];
        size_t pipe_pos = range.find('|');
        if (pipe_pos != std::string::npos && range[0] == '[' && range.back() == ']') {
          std::string min_str = range.substr(1, pipe_pos - 1);
          std::string max_str = range.substr(pipe_pos + 1, range.size() - pipe_pos - 2);
          env_var.min_value = std::stod(min_str);
          env_var.max_value = std::stod(max_str);
        }
        
        // Unit is the fourth token after EV_
        if (tokens[4][0] == '"' && tokens[4].back() == '"') {
          env_var.unit = tokens[4].substr(1, tokens[4].size() - 2);
        } else {
          env_var.unit = tokens[4];
        }
        
        // Initial value is the fifth token after EV_
        env_var.initial_value = std::stod(tokens[5]);
        
        // EV ID is the sixth token after EV_
        env_var.ev_id = std::stoi(tokens[6]);
        
        // Access type is the seventh token after EV_
        env_var.access_type = tokens[7];
        
        // Access nodes start at the eighth token after EV_ (if present)
        std::vector<std::string> access_nodes;
        if (tokens.size() > 8) {
          for (size_t i = 8; i < tokens.size(); ++i) {
            std::string node = tokens[i];
            // Remove trailing commas
            if (!node.empty() && node.back() == ',') {
              node.pop_back();
            }
            access_nodes.push_back(node);
          }
        }
        env_var.access_nodes = access_nodes;
        
        // Store in the environment_variables map with name as the key
        state.dbc_file.environment_variables[env_var.name] = env_var;
        state.found_valid_section = true;
        
        std::cout << "Successfully parsed environment variable: " << env_var.name << std::endl;
      } else {
        std::cout << "Failed to parse environment variable (invalid token count or format)" << std::endl;
      }
    } else {
      std::cout << "Empty environment variable content" << std::endl;
    }
  }
};

// Actions for ENVVAR_DATA_ section
template<>
struct action<grammar::env_var_data_line> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.set_env_var_data_content(in.string());
  }
};

template<>
struct action<grammar::env_var_data_section> {
  template<typename ActionInput>
  static void apply(const ActionInput&, dbc_state& state) {
    if (!state.env_var_data_content.empty()) {
      std::cout << "Original environment variable data content: '" << state.env_var_data_content << "'" << std::endl;
      
      // Parse environment variable data directly without using EnvironmentVariableDataParser
      std::string content = state.env_var_data_content;
      // Remove trailing newlines
      while (!content.empty() && (content.back() == '\n' || content.back() == '\r')) {
        content.pop_back();
      }
      
      // Expected format: ENVVAR_DATA_ EngineTemp: 5;
      // Split by spaces but handle colons and semicolons
      std::vector<std::string> tokens;
      std::string current_token;
      
      for (size_t i = 0; i < content.size(); ++i) {
        char c = content[i];
        
        if (c == ' ' || c == '\t') {
          if (!current_token.empty()) {
            tokens.push_back(current_token);
            current_token.clear();
          }
        } else if (c == ':') {
          if (!current_token.empty()) {
            tokens.push_back(current_token);
            current_token = ":";
          } else {
            current_token += c;
          }
        } else if (c == ';') {
          if (!current_token.empty()) {
            tokens.push_back(current_token);
            current_token = ";";
          } else {
            current_token += c;
          }
        } else {
          current_token += c;
        }
      }
      
      // Add the last token if not empty
      if (!current_token.empty()) {
        tokens.push_back(current_token);
      }
      
      // Debug output to see tokens
      std::cout << "Parsed " << tokens.size() << " tokens:" << std::endl;
      for (size_t i = 0; i < tokens.size(); ++i) {
        std::cout << "  " << i << ": '" << tokens[i] << "'" << std::endl;
      }
      
      // Extract values from tokens
      if (tokens.size() >= 4 && tokens[0] == "ENVVAR_DATA_") {
        std::string env_var_name;
        
        // Extract environment variable name (remove trailing colon if present)
        env_var_name = tokens[1];
        if (env_var_name.back() == ':') {
          env_var_name.pop_back();
        }
        
        // Update the DBC file only if we have a valid environment variable name
        if (!env_var_name.empty()) {
          // Create a new environment variable data entry
          DbcFile::EnvVarData env_var_data;
          env_var_data.data_name = env_var_name;
          
          // Store in the environment_variable_data map with name as the key
          state.dbc_file.environment_variable_data[env_var_name] = env_var_data;
          std::cout << "Created environment variable data for: " << env_var_name << std::endl;
          
          state.found_valid_section = true;
        } else {
          std::cout << "Failed to extract environment variable name" << std::endl;
        }
      } else {
        std::cout << "Failed to parse environment variable data (invalid token count or format)" << std::endl;
      }
    } else {
      std::cout << "Empty environment variable data content" << std::endl;
    }
  }
};

// Continuation line handling based on current section
template<>
struct action<grammar::indented_line> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    std::string content = in.string();
    
    // Handle continuation lines based on current section
    switch (state.current_section) {
      case dbc_state::SectionType::NewSymbols:
        state.add_to_new_symbols(content);
        break;
      case dbc_state::SectionType::BitTiming:
        state.add_to_bit_timing(content);
        break;
      case dbc_state::SectionType::Nodes:
        state.add_to_nodes(content);
        break;
      case dbc_state::SectionType::Message:
        state.add_to_message(content);
        break;
      case dbc_state::SectionType::MessageTransmitters:
        state.add_to_message_transmitters(content);
        break;
      case dbc_state::SectionType::ValueTable:
        state.add_to_value_table(content);
        break;
      case dbc_state::SectionType::EnvVar:
        state.add_to_env_var(content);
        break;
      case dbc_state::SectionType::EnvVarData:
        state.add_to_env_var_data(content);
        break;
      case dbc_state::SectionType::Comment:
        state.add_to_comment(content);
        break;
      case dbc_state::SectionType::AttrDef:
        state.add_to_attr_def(content);
        break;
      case dbc_state::SectionType::AttrDefDef:
        state.add_to_attr_def_def(content);
        break;
      case dbc_state::SectionType::Attr:
        state.add_to_attr(content);
        break;
      case dbc_state::SectionType::ValueDesc:
        state.add_to_value_desc(content);
        break;
      case dbc_state::SectionType::SigValType:
        state.add_to_sig_val_type(content);
        break;
      case dbc_state::SectionType::SigGroup:
        state.add_to_sig_group(content);
        break;
      case dbc_state::SectionType::SigMulVal:
        state.add_to_sig_mul_val(content);
        break;
      default:
        // VERSION doesn't support continuation lines
        break;
    }
  }
};

// Main parser implementation
std::optional<DbcFile> DbcFileParser::Parse(std::string_view input) {
  // Empty input check
  if (input.empty()) {
    return std::nullopt;
  }
  
  // Analyze grammar for potential issues
  if (const std::size_t issues = pegtl::analyze<grammar::dbc_file>()) {
    // We can still try to parse even if analysis shows issues
    std::cerr << "Grammar analysis found " << issues << " issues" << std::endl;
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
      
      // Direct processing of message transmitters since PEGTL grammar may not match them
      std::string input_str(input);
      std::istringstream iss(input_str);
      std::string line;
      
      while (std::getline(iss, line)) {
        if (line.find("BO_TX_BU_") != std::string::npos) {
          // Process this line directly
          auto transmitters_result = MessageTransmittersParser::Parse(line);
          if (transmitters_result) {
            state.dbc_file.message_transmitters[transmitters_result->message_id] = 
                transmitters_result->transmitters;
            state.found_valid_section = true;
          }
        }
      }
      
      // Return result if we found at least one valid section
      if (state.found_valid_section) {
        return state.dbc_file;
      }
    }
  } catch (const pegtl::parse_error& e) {
    // Handle parsing errors with detailed information
    std::cerr << "Parse error: " << e.what() << std::endl;
    return std::nullopt;
  }

  return std::nullopt;
}

}  // namespace parser
}  // namespace dbc_parser 