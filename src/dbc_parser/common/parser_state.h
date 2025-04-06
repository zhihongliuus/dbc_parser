#ifndef DBC_PARSER_PARSER_PARSER_STATE_H_
#define DBC_PARSER_PARSER_PARSER_STATE_H_

#include <optional>

namespace dbc_parser {
namespace parser {

/**
 * Base template class for parser state objects
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
   * Get the result of parsing
   * 
   * @return std::optional<ResultType> The parse result if valid, nullopt otherwise
   */
  [[nodiscard]] virtual std::optional<ResultType> GetResult() const noexcept {
    if (is_complete_) {
      return result_;
    }
    return std::nullopt;
  }

 protected:
  ResultType result_{};
  bool is_complete_ = false;
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_PARSER_STATE_H_ 