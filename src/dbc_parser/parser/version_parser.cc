#include "src/dbc_parser/parser/version_parser.h"

#include <optional>
#include <string>
#include <string_view>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/analyze.hpp"

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Grammar rules for VERSION parsing
namespace grammar {

// Basic whitespace rule
struct ws : pegtl::star<pegtl::space> {};

// VERSION keyword
struct version_keyword : pegtl::string<'V', 'E', 'R', 'S', 'I', 'O', 'N'> {};

// Rules for quoted strings with escaping
struct escaped_char : pegtl::seq<pegtl::one<'\\'>, pegtl::any> {};
struct regular_char : pegtl::not_one<'"', '\\'> {};
struct quoted_string_content : pegtl::star<pegtl::sor<escaped_char, regular_char>> {};
struct quoted_string : pegtl::seq<pegtl::one<'"'>, quoted_string_content, pegtl::one<'"'>> {};

// Complete VERSION rule
struct version_rule : pegtl::seq<ws, version_keyword, ws, quoted_string, ws, pegtl::eof> {};

} // namespace grammar

// Data structure to collect parsing results
struct version_state {
  std::string content;
  bool success = false;
};

// PEGTL actions
template<typename Rule>
struct version_action : pegtl::nothing<Rule> {};

// Action for extracting quoted string content
template<>
struct version_action<grammar::quoted_string_content> {
  template<typename ActionInput>
  static void apply(const ActionInput& in, version_state& state) {
    std::string content = in.string();
    
    // Process escape sequences
    std::string processed;
    processed.reserve(content.size());
    
    for (size_t i = 0; i < content.size(); ++i) {
      if (content[i] == '\\' && i + 1 < content.size()) {
        // Handle escape sequences
        if (content[i+1] == '"' || content[i+1] == '\\') {
          processed.push_back(content[i+1]);
          ++i;  // Skip the escaped character
        } else {
          // Keep both backslash and character for other escapes
          processed.push_back('\\');
          processed.push_back(content[i+1]);
          ++i;
        }
      } else {
        processed.push_back(content[i]);
      }
    }
    
    state.content = processed;
    state.success = true;
  }
};

std::optional<Version> VersionParser::Parse(std::string_view input) {
  // Create input for PEGTL parser
  pegtl::memory_input<> in(input.data(), input.size(), "VERSION");
  
  // Create state to collect results
  version_state state;
  
  try {
    // Parse input using our grammar and actions
    if (pegtl::parse<grammar::version_rule, version_action>(in, state) && 
        state.success && !state.content.empty()) {
      // Create and return Version object
      Version version;
      version.version = state.content;
      return version;
    }
  } catch (const pegtl::parse_error&) {
    // Parse error - return nullopt
    return std::nullopt;
  }
  
  return std::nullopt;
}

}  // namespace parser
}  // namespace dbc_parser 