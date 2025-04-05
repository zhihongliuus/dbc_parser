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
  static bool ValidateInput(std::string_view input) {
    return !input.empty();
  }
  
  /**
   * Protected helper method to create a PEGTL memory input
   * 
   * @param input The input string to parse
   * @param source_name A name for the input source (for error messages)
   * @return A PEGTL memory input object
   */
  static pegtl::memory_input<> CreateInput(std::string_view input, const std::string& source_name) {
    return pegtl::memory_input<>(input.data(), input.size(), source_name);
  }
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_PARSER_BASE_H_ 