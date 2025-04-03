#ifndef DBC_PARSER_SRC_GRAMMAR_DBC_GRAMMAR_H_
#define DBC_PARSER_SRC_GRAMMAR_DBC_GRAMMAR_H_

#include <string>
#include <vector>

namespace dbc_parser {
namespace grammar {

// Simple placeholder for future grammar implementation
class DbcGrammar {
 public:
  DbcGrammar() = default;
  ~DbcGrammar() = default;

  // Parse a DBC file content
  bool Parse(const std::string& content);
};

}  // namespace grammar
}  // namespace dbc_parser

#endif  // DBC_PARSER_SRC_GRAMMAR_DBC_GRAMMAR_H_ 