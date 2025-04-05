#ifndef SRC_DBC_PARSER_PARSER_PARSER_STATE_H_
#define SRC_DBC_PARSER_PARSER_PARSER_STATE_H_

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

  /**
   * Get the result of parsing
   * 
   * @return std::optional<ResultType> The parse result if valid, nullopt otherwise
   */
  virtual std::optional<ResultType> GetResult() const {
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

#endif  // SRC_DBC_PARSER_PARSER_PARSER_STATE_H_ 