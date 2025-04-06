#ifndef DBC_PARSER_CORE_STRING_UTILS_H_
#define DBC_PARSER_CORE_STRING_UTILS_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dbc_parser {
namespace core {

/**
 * @brief Utility class for string manipulation operations.
 *
 * StringUtils provides a collection of static methods for common string operations
 * such as trimming, splitting, case conversion, and parsing. All methods are
 * designed to be efficient and work with std::string_view where possible.
 */
class StringUtils {
 public:
  /**
   * @brief Trims whitespace from the beginning and end of a string.
   *
   * @param str The input string to trim
   * @return std::string A new string with leading and trailing whitespace removed
   */
  [[nodiscard]] static std::string Trim(std::string_view str);

  /**
   * @brief Splits a string by the given delimiter character.
   *
   * @param str The input string to split
   * @param delimiter The character to split on
   * @return std::vector<std::string> Vector containing the split substrings
   */
  [[nodiscard]] static std::vector<std::string> Split(std::string_view str, char delimiter);

  /**
   * @brief Splits a string by any of the provided delimiter characters.
   *
   * @param str The input string to split
   * @param delimiters String containing all possible delimiter characters
   * @return std::vector<std::string> Vector containing the split substrings
   */
  [[nodiscard]] static std::vector<std::string> SplitByAny(std::string_view str, std::string_view delimiters);

  /**
   * @brief Checks if a string contains valid UTF-8 characters.
   *
   * @param str The input string to validate
   * @return bool True if the string is valid UTF-8, false otherwise
   */
  [[nodiscard]] static bool IsValidUtf8(std::string_view str);

  /**
   * @brief Converts a string to uppercase.
   *
   * @param str The input string to convert
   * @return std::string A new string with all characters converted to uppercase
   */
  [[nodiscard]] static std::string ToUpper(std::string_view str);

  /**
   * @brief Converts a string to lowercase.
   *
   * @param str The input string to convert
   * @return std::string A new string with all characters converted to lowercase
   */
  [[nodiscard]] static std::string ToLower(std::string_view str);

  /**
   * @brief Extracts content between quotes, handling escaped quotes.
   *
   * Processes a quoted string (surrounded by double quotes) and handles
   * escape sequences like \" and \\.
   *
   * @param str The quoted string to process
   * @return std::optional<std::string> The content between quotes if properly quoted,
   *         std::nullopt if the input is not properly quoted
   */
  [[nodiscard]] static std::optional<std::string> ExtractQuoted(std::string_view str);

  /**
   * @brief Parses a string as an integer.
   *
   * @param str The string to parse
   * @return std::optional<int64_t> The parsed integer value if successful,
   *         std::nullopt if parsing fails
   */
  [[nodiscard]] static std::optional<int64_t> ParseInt(std::string_view str);

  /**
   * @brief Parses a string as a floating point number.
   *
   * @param str The string to parse
   * @return std::optional<double> The parsed double value if successful,
   *         std::nullopt if parsing fails
   */
  [[nodiscard]] static std::optional<double> ParseDouble(std::string_view str);

  /**
   * @brief Checks if a string starts with a given prefix.
   *
   * @param str The string to check
   * @param prefix The prefix to look for
   * @return bool True if the string starts with the prefix, false otherwise
   */
  [[nodiscard]] static bool StartsWith(std::string_view str, std::string_view prefix);

  /**
   * @brief Checks if a string ends with a given suffix.
   *
   * @param str The string to check
   * @param suffix The suffix to look for
   * @return bool True if the string ends with the suffix, false otherwise
   */
  [[nodiscard]] static bool EndsWith(std::string_view str, std::string_view suffix);

  /**
   * @brief Removes quotes from a quoted string if present.
   *
   * If the string starts and ends with double quotes, they are removed.
   * Unlike ExtractQuoted, this does not process escape sequences.
   *
   * @param str The string to process
   * @return std::string The string without surrounding quotes, or the original string if not quoted
   */
  [[nodiscard]] static std::string StripQuotes(std::string_view str);

  /**
   * @brief Joins a vector of strings with a delimiter.
   *
   * @param parts Vector of strings to join
   * @param delimiter The delimiter to insert between parts
   * @return std::string The joined string
   */
  [[nodiscard]] static std::string Join(const std::vector<std::string>& parts, std::string_view delimiter);
};

}  // namespace core
}  // namespace dbc_parser

#endif  // DBC_PARSER_CORE_STRING_UTILS_H_ 