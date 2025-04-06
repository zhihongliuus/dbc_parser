#ifndef DBC_PARSER_PARSER_NEW_SYMBOLS_PARSER_H_
#define DBC_PARSER_PARSER_NEW_SYMBOLS_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

/**
 * @brief Structure to hold new symbols information.
 *
 * Contains the list of symbols defined in the NS_ section of a DBC file.
 * These symbols define the types of objects and attributes allowed in the DBC file.
 */
struct NewSymbols {
  std::vector<std::string> symbols;  ///< List of new symbols defined in the DBC file
};

/**
 * @brief Parser for the NS_ (New Symbols) section in DBC files.
 *
 * Handles parsing of the NS_ section, which defines the symbols used throughout the DBC file.
 * This section typically appears near the beginning of a DBC file and lists identifiers 
 * for various elements like messages, signals, and attributes.
 *
 * Example DBC new symbols format:
 * NS_ :
 *   NS_DESC_
 *   CM_
 *   BA_DEF_
 *   BA_
 *   VAL_
 *   CAT_DEF_
 *   ...
 */
class NewSymbolsParser : public ParserBase {
 public:
  /**
   * @brief Parses an NS_ section from the given input string.
   *
   * Takes a string containing a DBC NS_ section and parses it into a NewSymbols object.
   * The parser validates the syntax and extracts the list of symbol identifiers.
   *
   * @param input String view containing the new symbols section to parse
   * @return std::optional<NewSymbols> A NewSymbols object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<NewSymbols> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_NEW_SYMBOLS_PARSER_H_ 