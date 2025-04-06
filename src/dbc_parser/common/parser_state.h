#ifndef DBC_PARSER_PARSER_PARSER_STATE_H_
#define DBC_PARSER_PARSER_PARSER_STATE_H_

#include <optional>

namespace dbc_parser {
namespace parser {

/**
 * @brief Base template class for parser state objects.
 * 
 * This class provides a common interface for maintaining state during parsing operations.
 * Parser state objects store intermediate and final results of parsing operations,
 * and track whether parsing has been successfully completed.
 * 
 * @tparam ResultType The type of object returned by the parser
 */
template <typename ResultType>
class ParserState {
 public:
  ParserState() = default;
  virtual ~ParserState() = default;

  // Prevent copying and moving
  ParserState(const ParserState&) = delete;
  ParserState& operator=(const ParserState&) = delete;
  ParserState(ParserState&&) = delete;
  ParserState& operator=(ParserState&&) = delete;

  /**
   * @brief Get the result of parsing.
   * 
   * Returns the parsed result if parsing has been successfully completed,
   * or std::nullopt if parsing is incomplete or failed.
   * 
   * @return std::optional<ResultType> The parse result if valid, std::nullopt otherwise
   */
  [[nodiscard]] virtual std::optional<ResultType> GetResult() const noexcept {
    if (is_complete_) {
      return result_;
    }
    return std::nullopt;
  }

 protected:
  ResultType result_{};       ///< Stores the result of the parsing operation
  bool is_complete_ = false;  ///< Indicates whether parsing has been successfully completed
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_PARSER_STATE_H_ 