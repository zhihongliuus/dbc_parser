#ifndef SRC_DBC_PARSER_PARSER_PARSER_BASE_H_
#define SRC_DBC_PARSER_PARSER_PARSER_BASE_H_

#include <optional>
#include <string>
#include <string_view>
#include <tao/pegtl.hpp>

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

/**
 * Base class for parsers with common functionality
 */
class ParserBase {
 public:
  /**
   * Utility to unescape quoted string content
   * 
   * @param quoted The quoted string to unescape
   * @return The unescaped string content without quotes
   */
  [[nodiscard]] static std::string UnescapeString(std::string_view quoted) noexcept {
    if (quoted.size() < 2) return "";
    
    // Remove surrounding quotes
    std::string_view content = quoted.substr(1, quoted.size() - 2);
    
    std::string result;
    result.reserve(content.size());
    
    bool escaped = false;
    for (char c : content) {
      if (escaped) {
        result.push_back(c);
        escaped = false;
      } else if (c == '\\') {
        escaped = true;
      } else {
        result.push_back(c);
      }
    }
    
    return result;
  }

 protected:
  /**
   * Constructor is protected since this is a base class
   */
  ParserBase() = default;
  
  /**
   * Protected virtual destructor for proper cleanup in derived classes
   */
  virtual ~ParserBase() = default;
  
  /**
   * Protected helper method to validate that input is not empty
   * 
   * @param input The input string to validate
   * @return true if input is not empty, false otherwise
   */
  [[nodiscard]] static bool ValidateInput(std::string_view input) noexcept {
    return !input.empty();
  }
  
  /**
   * Protected helper method to create a PEGTL memory input
   * 
   * @param input The input string to parse
   * @param source_name A name for the input source (for error messages)
   * @return A PEGTL memory input object
   */
  [[nodiscard]] static pegtl::memory_input<> CreateInput(
      std::string_view input, const std::string& source_name) {
    return pegtl::memory_input<>(input.data(), input.size(), source_name);
  }
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_PARSER_BASE_H_ 