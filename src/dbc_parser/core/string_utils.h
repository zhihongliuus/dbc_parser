#ifndef DBC_PARSER_CORE_STRING_UTILS_H_
#define DBC_PARSER_CORE_STRING_UTILS_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dbc_parser {
namespace core {

class StringUtils {
 public:
  // Trims whitespace from the beginning and end of a string.
  [[nodiscard]] static std::string Trim(std::string_view str);

  // Splits a string by the given delimiter.
  [[nodiscard]] static std::vector<std::string> Split(std::string_view str, char delimiter);

  // Splits a string by multiple delimiters.
  [[nodiscard]] static std::vector<std::string> SplitByAny(std::string_view str, std::string_view delimiters);

  // Checks if a string contains valid UTF-8 characters.
  [[nodiscard]] static bool IsValidUtf8(std::string_view str);

  // Converts a string to uppercase.
  [[nodiscard]] static std::string ToUpper(std::string_view str);

  // Converts a string to lowercase.
  [[nodiscard]] static std::string ToLower(std::string_view str);

  // Extracts content between quotes, handling escaped quotes.
  // Returns empty optional if the string is not properly quoted.
  [[nodiscard]] static std::optional<std::string> ExtractQuoted(std::string_view str);

  // Tries to parse a string as an integer.
  // Returns empty optional if parsing fails.
  [[nodiscard]] static std::optional<int64_t> ParseInt(std::string_view str);

  // Tries to parse a string as a floating point number.
  // Returns empty optional if parsing fails.
  [[nodiscard]] static std::optional<double> ParseDouble(std::string_view str);

  // Checks if a string starts with a given prefix.
  [[nodiscard]] static bool StartsWith(std::string_view str, std::string_view prefix);

  // Checks if a string ends with a given suffix.
  [[nodiscard]] static bool EndsWith(std::string_view str, std::string_view suffix);

  // Removes quotes from a quoted string if present.
  // Returns the original string if not quoted.
  [[nodiscard]] static std::string StripQuotes(std::string_view str);

  // Joins a vector of strings with a delimiter.
  [[nodiscard]] static std::string Join(const std::vector<std::string>& parts, std::string_view delimiter);
};

}  // namespace core
}  // namespace dbc_parser

#endif  // DBC_PARSER_CORE_STRING_UTILS_H_ 