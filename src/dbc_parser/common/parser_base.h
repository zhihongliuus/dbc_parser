#ifndef DBC_PARSER_PARSER_PARSER_BASE_H_
#define DBC_PARSER_PARSER_PARSER_BASE_H_

#include <optional>
#include <string>
#include <string_view>
#include <tao/pegtl.hpp>

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

/**
 * @brief Base class for all DBC file parsers.
 *
 * Provides common utilities and abstract interface for specific parser implementations.
 * All parsers in the DBC Parser library inherit from this base class to maintain
 * consistent behavior and interface.
 */
class ParserBase {
 public:
  /**
   * @brief Unescapes a quoted string by processing escape sequences.
   *
   * Handles escape sequences like \" and \\ in quoted strings and returns the 
   * content without surrounding quotes. The function assumes that the input 
   * string includes the surrounding quotes.
   * 
   * @param quoted The quoted string to unescape (including quotes)
   * @return std::string The unescaped string content without quotes
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
   * @brief Default constructor is protected since this is a base class.
   *
   * Prevents direct instantiation of the ParserBase class.
   */
  ParserBase() = default;
  
  /**
   * @brief Virtual destructor for proper cleanup in derived classes.
   */
  virtual ~ParserBase() = default;
  
  /**
   * @brief Validates that input is not empty.
   *
   * A basic validation step common to all parsers to ensure they have content to parse.
   * 
   * @param input The input string to validate
   * @return bool true if input is not empty, false otherwise
   */
  [[nodiscard]] static bool ValidateInput(std::string_view input) noexcept {
    return !input.empty();
  }
  
  /**
   * @brief Creates a PEGTL memory input object for parsing.
   *
   * Wraps the input string in a PEGTL memory input that can be used with PEGTL parsers.
   * 
   * @param input The input string to parse
   * @param source_name A name for the input source (for error reporting)
   * @return pegtl::memory_input<> A PEGTL memory input object ready for parsing
   */
  [[nodiscard]] static pegtl::memory_input<> CreateInput(
      std::string_view input, const std::string& source_name) noexcept {
    return pegtl::memory_input<>(input.data(), input.size(), source_name);
  }
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_PARSER_BASE_H_ 