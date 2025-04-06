#ifndef DBC_PARSER_PARSER_NEW_SYMBOLS_PARSER_H_
#define DBC_PARSER_PARSER_NEW_SYMBOLS_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

// Structure to hold new symbols information
struct NewSymbols {
  std::vector<std::string> symbols;
};

// Parser for DBC NS_ (New Symbols) section
class NewSymbolsParser : public ParserBase {
 public:
  // Parses an NS_ string and returns a NewSymbols object if successful
  [[nodiscard]] static std::optional<NewSymbols> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_NEW_SYMBOLS_PARSER_H_ 