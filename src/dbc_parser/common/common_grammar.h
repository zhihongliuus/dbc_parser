#ifndef DBC_PARSER_PARSER_COMMON_GRAMMAR_H_
#define DBC_PARSER_PARSER_COMMON_GRAMMAR_H_

#include <tao/pegtl.hpp>

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

/**
 * @brief Common grammar rules used across parsers in the DBC parser library.
 *
 * This namespace provides reusable grammar components based on the PEGTL library
 * for parsing different elements of DBC files. These rules form the building blocks
 * for the more complex parsers in the library.
 */
namespace common_grammar {

// Basic whitespace and EOL rules
/** @brief Matches zero or more whitespace characters */
struct ws : pegtl::star<pegtl::space> {};
/** @brief Matches end of line */
struct eol : pegtl::eol {};
/** @brief Matches end of line or end of file */
struct eolf : pegtl::eolf {};

// Basic character categories
/** @brief Matches a single digit [0-9] */
struct digit : pegtl::digit {};
/** @brief Matches a single letter [a-zA-Z] */
struct alpha : pegtl::alpha {};
/** @brief Matches an identifier character (letter, digit, or underscore) */
struct id_char : pegtl::sor<alpha, digit, pegtl::one<'_'>> {};

// Whitespace and semicolon
/** @brief Matches a semicolon */
struct semicolon : pegtl::one<';'> {};
/** @brief Matches zero or more whitespace characters */
struct opt_ws : pegtl::star<pegtl::space> {};
/** @brief Matches one or more whitespace characters */
struct req_ws : pegtl::plus<pegtl::space> {};

// Rules for quoted strings with escaping
/** @brief Matches an escaped character (backslash followed by any character) */
struct escaped_char : pegtl::seq<pegtl::one<'\\'>, pegtl::any> {};
/** @brief Matches any character except double quote and backslash */
struct regular_char : pegtl::not_one<'"', '\\'> {};
/** @brief Matches string content (zero or more regular or escaped characters) */
struct string_content : pegtl::star<pegtl::sor<escaped_char, regular_char>> {};
/** @brief Matches a quoted string (content surrounded by double quotes) */
struct quoted_string : pegtl::seq<pegtl::one<'"'>, string_content, pegtl::one<'"'>> {};

// Rules for numeric values
/** @brief Matches an optional + or - sign */
struct sign : pegtl::opt<pegtl::one<'+', '-'>> {};
/** @brief Matches a decimal point */
struct dot : pegtl::one<'.'> {};
/** @brief Matches one or more digits */
struct digits : pegtl::plus<digit> {};
/** @brief Matches an integer (optional sign followed by digits) */
struct integer : pegtl::seq<sign, digits> {};
/** @brief Matches a floating point number (with decimal point) */
struct floating_point : pegtl::seq<
                           sign,
                           pegtl::opt<digits>,
                           dot,
                           pegtl::opt<digits>
                         > {};

// Message ID used in various places (can be signed)
/** @brief Matches a CAN message ID (optional sign followed by digits) */
struct message_id : pegtl::seq<sign, digits> {};

// Common identifiers
/** @brief Matches a standard identifier */
struct identifier : pegtl::identifier {};

// Quoted and unquoted identifiers (for objects that can have either)
/** @brief Matches an identifier within double quotes */
struct quoted_identifier : quoted_string {};
/** @brief Matches an unquoted identifier */
struct unquoted_identifier : pegtl::identifier {};

// Common DBC keywords
/** @brief Matches the "VERSION" keyword */
struct version_keyword : pegtl::string<'V', 'E', 'R', 'S', 'I', 'O', 'N'> {};
/** @brief Matches the "NS_" keyword (New Symbols) */
struct ns_keyword : pegtl::string<'N', 'S', '_'> {};
/** @brief Matches the "BS_" keyword (Bit Timing) */
struct bs_keyword : pegtl::string<'B', 'S', '_'> {};
/** @brief Matches the "BU_" keyword (Nodes) */
struct bu_keyword : pegtl::string<'B', 'U', '_'> {};
/** @brief Matches the "BO_" keyword (Message) */
struct bo_keyword : pegtl::string<'B', 'O', '_'> {};
/** @brief Matches the "SG_" keyword (Signal) */
struct sg_keyword : pegtl::string<'S', 'G', '_'> {};
/** @brief Matches the "EV_" keyword (Environment Variable) */
struct ev_keyword : pegtl::string<'E', 'V', '_'> {};
/** @brief Matches the "CM_" keyword (Comment) */
struct cm_keyword : pegtl::string<'C', 'M', '_'> {};
/** @brief Matches the "BA_DEF_" keyword (Attribute Definition) */
struct ba_def_keyword : pegtl::string<'B', 'A', '_', 'D', 'E', 'F', '_'> {};
/** @brief Matches the "BA_DEF_DEF_" keyword (Attribute Definition Default) */
struct ba_def_def_keyword : pegtl::string<'B', 'A', '_', 'D', 'E', 'F', '_', 'D', 'E', 'F', '_'> {};
/** @brief Matches the "BA_" keyword (Attribute Value) */
struct ba_keyword : pegtl::string<'B', 'A', '_'> {};
/** @brief Matches the "VAL_" keyword (Value Description) */
struct val_keyword : pegtl::string<'V', 'A', 'L', '_'> {};
/** @brief Matches the "VAL_TABLE_" keyword (Value Table) */
struct val_table_keyword : pegtl::string<'V', 'A', 'L', '_', 'T', 'A', 'B', 'L', 'E', '_'> {};
/** @brief Matches the "ENVVAR_DATA_" keyword (Environment Variable Data) */
struct envvar_data_keyword : pegtl::string<'E', 'N', 'V', 'V', 'A', 'R', '_', 'D', 'A', 'T', 'A', '_'> {};

// Punctuation used in several parsers
/** @brief Matches a colon */
struct colon : pegtl::one<':'> {};
/** @brief Matches a comma */
struct comma : pegtl::one<','> {};
/** @brief Matches a pipe character */
struct pipe : pegtl::one<'|'> {};
/** @brief Matches an at sign */
struct at_sign : pegtl::one<'@'> {};
/** @brief Matches a left square bracket */
struct lbracket : pegtl::one<'['> {};
/** @brief Matches a right square bracket */
struct rbracket : pegtl::one<']'> {};
/** @brief Matches a left parenthesis */
struct lparen : pegtl::one<'('> {};
/** @brief Matches a right parenthesis */
struct rparen : pegtl::one<')'> {};

}  // namespace common_grammar
}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_COMMON_GRAMMAR_H_ 