#include "src/dbc_parser/parser/dbc_file_parser.h"

#include <iostream>
#include <string>
#include <string_view>
#include <memory>
#include <optional>
#include <map>
#include <utility>
#include <variant>
#include <sstream>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>

#include "src/dbc_parser/core/string_utils.h"
#include "src/dbc_parser/parser/version_parser.h"
#include "src/dbc_parser/parser/new_symbols_parser.h"
#include "src/dbc_parser/parser/nodes_parser.h"
#include "src/dbc_parser/parser/message_parser.h"
#include "src/dbc_parser/parser/message_transmitters_parser.h"
#include "src/dbc_parser/parser/bit_timing_parser.h"
#include "src/dbc_parser/parser/value_table_parser.h"
#include "src/dbc_parser/parser/environment_variable_parser.h"
#include "src/dbc_parser/parser/environment_variable_data_parser.h"
#include "src/dbc_parser/parser/comment_parser.h"
#include "src/dbc_parser/parser/signal_value_type_parser.h"
#include "src/dbc_parser/parser/signal_group_parser.h"


// Only include parsers that are actually used in this file
// Other parsers are included in the BUILD file for future implementation

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Use string utilities from the core namespace
using dbc_parser::core::StringUtils;

// Helper for string operations
class StringUtilities {
public:
  // Split a string by a delimiter character and return a vector of trimmed tokens
  static std::vector<std::string> SplitTrimmed(const std::string& str, char delimiter) {
    std::vector<std::string> tokens = StringUtils::Split(str, delimiter);
    for (auto& token : tokens) {
      token = StringUtils::Trim(token);
    }
    return tokens;
  }
  
  // Split a string by multiple delimiters and return a vector of trimmed tokens
  static std::vector<std::string> SplitTrimmedByAny(const std::string& str, const std::string& delimiters) {
    std::vector<std::string> tokens = StringUtils::SplitByAny(str, delimiters);
    for (auto& token : tokens) {
      token = StringUtils::Trim(token);
    }
    return tokens;
  }
  
  // Trim whitespace from start and end of string
  static std::string Trim(const std::string& str) {
    return StringUtils::Trim(str);
  }
  
  // Extract string content between quotes
  static std::optional<std::string> ExtractQuoted(const std::string& str) {
    return StringUtils::ExtractQuoted(str);
  }
  
  // Remove quotes from a string if present
  static std::string StripQuotes(const std::string& str) {
    return StringUtils::StripQuotes(str);
  }
  
  // Parse a string as an integer
  static std::optional<int64_t> ParseInt(const std::string& str) {
    return StringUtils::ParseInt(str);
  }
  
  // Parse a string as a double
  static std::optional<double> ParseDouble(const std::string& str) {
    return StringUtils::ParseDouble(str);
  }
  
  // Join a vector of strings with a delimiter
  static std::string Join(const std::vector<std::string>& parts, const std::string& delimiter) {
    return StringUtils::Join(parts, delimiter);
  }
};

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

// Enhanced rules for parsing specific data
struct message_id : pegtl::plus<pegtl::digit> {};
struct node_list : pegtl::list<pegtl::identifier, pegtl::one<','>, pegtl::space> {};

// Section rules with content capturing
struct version_section : pegtl::sor<version_content, invalid_version_content> {};
struct new_symbols_line : pegtl::seq<new_symbols_key, line_content> {};
struct nodes_line : pegtl::seq<nodes_key, line_content> {};
struct message_line : pegtl::seq<message_key, line_content> {};

// Enhanced message transmitters rule with improved pattern matching
struct message_transmitters_content : pegtl::seq<
                                       message_transmitters_key,
                                       ws,
                                       message_id,  // Message ID
                                       ws,
                                       node_list,  // List of transmitting nodes
                                       pegtl::opt<ws>,
                                       pegtl::one<';'>,  // Required semicolon
                                       pegtl::opt<ws>,
                                       pegtl::eol
                                     > {};

// Special handling for message transmitters with semicolon
struct message_transmitters_line : pegtl::seq<message_transmitters_key, pegtl::until<pegtl::eol>> {};
struct message_transmitters_section : pegtl::sor<message_transmitters_content, message_transmitters_line> {};

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
                      message_transmitters_section,
                      env_var_section,
                      env_var_data_section,
                      comment_section,
                      signal_key,
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
        for (const auto& node : *nodes_result) {
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
        
        // Create a detailed message definition
        DbcFile::MessageDef msg_def;
        msg_def.id = message_result->id;
        msg_def.name = message_result->name;
        msg_def.size = message_result->dlc;  // DLC corresponds to size
        msg_def.transmitter = message_result->sender;  // Sender corresponds to transmitter
        
        // Copy signals from the parsed message
        msg_def.signals = message_result->signals;
        
        // Store the detailed message in the messages_detailed map
        state.dbc_file.messages_detailed[message_result->id] = msg_def;
        
        // Update current message ID for signal association
        state.current_message_id = message_result->id;
        state.found_valid_section = true;
      }
    }
  }
};

// Actions for BO_TX_BU_ section
template<>
struct action<grammar::message_transmitters_content> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    std::string content = in.string();
    
    // Parse the message transmitters directly through the dedicated parser
    auto transmitters_result = MessageTransmittersParser::Parse(content);
    if (transmitters_result) {
      state.dbc_file.message_transmitters[transmitters_result->message_id] = 
          transmitters_result->transmitters;
      state.found_valid_section = true;
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
      auto bit_timing_result = BitTimingParser::Parse(state.bit_timing_content);
      if (bit_timing_result) {
        // Set the bit timing data in the DbcFile
        DbcFile::BitTiming bit_timing;
        bit_timing.baudrate = bit_timing_result->baudrate;
        // BitTimingParser uses a combined btr1_btr2 field
        // We need to adapt it to the DbcFile::BitTiming struct which has separate fields
        
        // Note: This is an approximation as we don't have the exact algorithm to split the combined value
        // A more precise implementation would use the correct splitting logic
        bit_timing.btr1 = static_cast<int>(bit_timing_result->btr1_btr2) / 100;
        bit_timing.btr2 = static_cast<int>(bit_timing_result->btr1_btr2) % 100;
        
        state.dbc_file.bit_timing = bit_timing;
        state.found_valid_section = true;
      }
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
template<>
struct action<grammar::signal_key> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    // Only process signals if we have a current message ID
    if (state.current_message_id >= 0) {
      std::string signal_line = in.string();
      
      // First, check if the line contains a signal definition
      if (signal_line.find("SG_") != std::string::npos) {
        state.found_valid_section = true;
        
        // The signals are now parsed via MessageParser in the message_section action
        // This handler is just for detecting signal sections for validation
        // If we later need to parse individual signals outside of a message context, we would:
        // 1. Include signal_parser.h properly
        // 2. Use SignalParser::Parse to parse individual signals
        // 3. Associate them with the current message
      }
    }
  }
};

// Actions for SIG_VALTYPE_ section
template<>
struct action<grammar::sig_val_type_line> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.set_sig_val_type_content(in.string());
  }
};

template<>
struct action<grammar::sig_val_type_section> {
  template<typename ActionInput>
  static void apply(const ActionInput&, dbc_state& state) {
    if (!state.sig_val_type_content.empty()) {
      auto sig_val_type_result = SignalValueTypeParser::Parse(state.sig_val_type_content);
      if (sig_val_type_result) {
        // Create a new signal value type definition
        DbcFile::SignalValueType sig_val_type;
        sig_val_type.message_id = sig_val_type_result->message_id;
        sig_val_type.signal_name = sig_val_type_result->signal_name;
        sig_val_type.value_type = sig_val_type_result->type;
        
        // Add the signal value type to the list
        state.dbc_file.signal_value_types.push_back(sig_val_type);
        state.found_valid_section = true;
      }
    }
  }
};

// Actions for SIG_GROUP_ section
template<>
struct action<grammar::sig_group_line> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.set_sig_group_content(in.string());
  }
};

template<>
struct action<grammar::sig_group_section> {
  template<typename ActionInput>
  static void apply(const ActionInput&, dbc_state& state) {
    if (!state.sig_group_content.empty()) {
      // Trim any trailing newlines and whitespace
      std::string content = StringUtilities::Trim(state.sig_group_content);
      
      auto sig_group_result = SignalGroupParser::Parse(content);
      if (sig_group_result) {
        // Create a new signal group definition
        DbcFile::SignalGroupDef sig_group;
        sig_group.message_id = sig_group_result->message_id;
        sig_group.name = sig_group_result->group_name;
        sig_group.repetitions = sig_group_result->repetitions;
        sig_group.signal_names = sig_group_result->signals;
        
        // Add the signal group to the list
        state.dbc_file.signal_groups.push_back(sig_group);
        state.found_valid_section = true;
      }
    }
  }
};

// Actions for CM_ section
template<>
struct action<grammar::comment_line> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.set_comment_content(in.string());
  }
};

template<>
struct action<grammar::comment_section> {
  template<typename ActionInput>
  static void apply(const ActionInput&, dbc_state& state) {
    if (!state.comment_content.empty()) {
      std::cout << "Original comment content: '" << state.comment_content << "'" << std::endl;
      
      // Trim any trailing newlines and whitespace
      std::string content = StringUtilities::Trim(state.comment_content);
      std::cout << "Cleaned comment content: '" << content << "'" << std::endl;
      
      // Use the existing CommentParser
      auto comment_result = CommentParser::Parse(content);
      
      if (comment_result) {
        // Create a new comment definition
        DbcFile::CommentDef comment_def;
        
        // Set the comment text
        comment_def.text = comment_result->text;
        
        // Set the comment type and associated object based on the comment type
        if (comment_result->type == CommentType::NETWORK) {
          comment_def.type = DbcFile::CommentDef::Type::Network;
        } else if (comment_result->type == CommentType::NODE) {
          comment_def.type = DbcFile::CommentDef::Type::Node;
          if (std::holds_alternative<std::string>(comment_result->identifier)) {
            comment_def.object_name = std::get<std::string>(comment_result->identifier);
          }
        } else if (comment_result->type == CommentType::MESSAGE) {
          comment_def.type = DbcFile::CommentDef::Type::Message;
          if (std::holds_alternative<int>(comment_result->identifier)) {
            comment_def.object_id = std::get<int>(comment_result->identifier);
          }
        } else if (comment_result->type == CommentType::SIGNAL) {
          comment_def.type = DbcFile::CommentDef::Type::Signal;
          if (std::holds_alternative<std::pair<int, std::string>>(comment_result->identifier)) {
            const auto& id_pair = std::get<std::pair<int, std::string>>(comment_result->identifier);
            comment_def.object_id = id_pair.first;
            comment_def.signal_index = 0;  // We don't have signal indices yet, set to 0
            comment_def.object_name = id_pair.second;  // Store signal name in object_name
          }
        } else if (comment_result->type == CommentType::ENV_VAR) {
          comment_def.type = DbcFile::CommentDef::Type::EnvVar;
          if (std::holds_alternative<std::string>(comment_result->identifier)) {
            comment_def.object_name = std::get<std::string>(comment_result->identifier);
          }
        }
        
        // Add the comment to the list
        state.dbc_file.comments.push_back(comment_def);
        state.found_valid_section = true;
        
        std::cout << "Successfully parsed comment of type: " 
                  << static_cast<int>(comment_def.type) << " with text: '" << comment_def.text << "'" << std::endl;
      } else {
        std::cout << "Failed to parse comment" << std::endl;
      }
    } else {
      std::cout << "Empty comment content" << std::endl;
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

// Actions for env_var sections
template<>
struct action<grammar::env_var_line> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.set_env_var_content(in.string());
    std::cout << "Setting environment variable content: '" << in.string() << "'" << std::endl;
  }
};

// Actions for env_var_data sections
template<>
struct action<grammar::env_var_data_line> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, dbc_state& state) {
    state.set_env_var_data_content(in.string());
    std::cout << "Setting environment variable data content: '" << in.string() << "'" << std::endl;
  }
};

template<>
struct action<grammar::env_var_data_section> {
  template<typename ActionInput>
  static void apply(const ActionInput&, dbc_state& state) {
    if (!state.env_var_data_content.empty()) {
      std::cout << "Original environment variable data content: '" << state.env_var_data_content << "'" << std::endl;
      
      // Trim any trailing newlines and whitespace
      std::string content = StringUtilities::Trim(state.env_var_data_content);
      std::cout << "Cleaned environment variable data content: '" << content << "'" << std::endl;
      
      // Use the existing EnvironmentVariableDataParser
      auto env_var_data_result = EnvironmentVariableDataParser::Parse(content);
      
      if (env_var_data_result) {
        // Create a new environment variable data entry
        DbcFile::EnvVarData env_var_data;
        env_var_data.data_name = env_var_data_result->name;
        
        // Store in the environment_variable_data map with name as the key
        state.dbc_file.environment_variable_data[env_var_data.data_name] = env_var_data;
        std::cout << "Created environment variable data for: " << env_var_data.data_name << std::endl;
        
        state.found_valid_section = true;
      } else {
        std::cout << "Failed to parse environment variable data" << std::endl;
      }
    } else {
      std::cout << "Empty environment variable data content" << std::endl;
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

template<>
struct action<grammar::env_var_section> {
  template<typename ActionInput>
  static void apply(const ActionInput&, dbc_state& state) {
    if (!state.env_var_content.empty()) {
      std::cout << "Original environment variable content: '" << state.env_var_content << "'" << std::endl;
      
      // Trim any trailing newlines and whitespace
      std::string content = StringUtilities::Trim(state.env_var_content);
      std::cout << "Cleaned environment variable content: '" << content << "'" << std::endl;
      
      // Use the existing EnvironmentVariableParser
      auto env_var_result = EnvironmentVariableParser::Parse(content);
      
      if (env_var_result) {
        // Create a new environment variable entry
        DbcFile::EnvVar env_var;
        env_var.name = env_var_result->name;
        env_var.type = env_var_result->var_type;
        env_var.min_value = env_var_result->minimum;
        env_var.max_value = env_var_result->maximum;
        env_var.unit = env_var_result->unit;
        env_var.initial_value = env_var_result->initial_value;
        env_var.ev_id = env_var_result->ev_id;
        env_var.access_type = env_var_result->access_type;
        
        // Convert access_nodes string to vector using the utility class
        env_var.access_nodes = StringUtilities::SplitTrimmed(env_var_result->access_nodes, ',');
        
        // Store in the environment_variables map with name as the key
        state.dbc_file.environment_variables[env_var.name] = env_var;
        state.found_valid_section = true;
        
        std::cout << "Successfully parsed environment variable: " << env_var.name << std::endl;
      } else {
        std::cout << "Failed to parse environment variable" << std::endl;
      }
    } else {
      std::cout << "Empty environment variable content" << std::endl;
    }
  }
};

}  // namespace parser
}  // namespace dbc_parser 