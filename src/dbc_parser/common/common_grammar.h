#ifndef SRC_DBC_PARSER_PARSER_COMMON_GRAMMAR_H_
#define SRC_DBC_PARSER_PARSER_COMMON_GRAMMAR_H_

#include <tao/pegtl.hpp>

namespace dbc_parser {
namespace parser {

namespace pegtl = tao::pegtl;

// Common grammar rules used across parsers
namespace common_grammar {

// Basic whitespace rule
struct ws : pegtl::star<pegtl::space> {};

// End of line rules
struct eol : pegtl::eol {};
struct eolf : pegtl::eolf {};

// Basic character categories
struct digit : pegtl::digit {};
struct alpha : pegtl::alpha {};
struct id_char : pegtl::sor<alpha, digit, pegtl::one<'_'>> {};

// Whitespace and semicolon
struct semicolon : pegtl::one<';'> {};
struct opt_ws : pegtl::star<pegtl::space> {};
struct req_ws : pegtl::plus<pegtl::space> {};

// Rules for quoted strings with escaping
struct escaped_char : pegtl::seq<pegtl::one<'\\'>, pegtl::any> {};
struct regular_char : pegtl::not_one<'"', '\\'> {};
struct string_content : pegtl::star<pegtl::sor<escaped_char, regular_char>> {};
struct quoted_string : pegtl::seq<pegtl::one<'"'>, string_content, pegtl::one<'"'>> {};

// Rules for numeric values
struct sign : pegtl::opt<pegtl::one<'+', '-'>> {};
struct dot : pegtl::one<'.'> {};
struct digits : pegtl::plus<digit> {};
struct integer : pegtl::seq<sign, digits> {};
struct floating_point : pegtl::seq<
                           sign,
                           pegtl::opt<digits>,
                           dot,
                           pegtl::opt<digits>
                         > {};

// Message ID used in various places (can be signed)
struct message_id : pegtl::seq<sign, digits> {};

// Common identifiers
struct identifier : pegtl::identifier {};

// Quoted and unquoted identifiers (for objects that can have either)
struct quoted_identifier : quoted_string {};
struct unquoted_identifier : pegtl::identifier {};

// Common DBC keywords
struct version_keyword : pegtl::string<'V', 'E', 'R', 'S', 'I', 'O', 'N'> {};
struct ns_keyword : pegtl::string<'N', 'S', '_'> {};
struct bs_keyword : pegtl::string<'B', 'S', '_'> {};
struct bu_keyword : pegtl::string<'B', 'U', '_'> {};
struct bo_keyword : pegtl::string<'B', 'O', '_'> {};
struct sg_keyword : pegtl::string<'S', 'G', '_'> {};
struct ev_keyword : pegtl::string<'E', 'V', '_'> {};
struct cm_keyword : pegtl::string<'C', 'M', '_'> {};
struct ba_def_keyword : pegtl::string<'B', 'A', '_', 'D', 'E', 'F', '_'> {};
struct ba_def_def_keyword : pegtl::string<'B', 'A', '_', 'D', 'E', 'F', '_', 'D', 'E', 'F', '_'> {};
struct ba_keyword : pegtl::string<'B', 'A', '_'> {};
struct val_keyword : pegtl::string<'V', 'A', 'L', '_'> {};
struct val_table_keyword : pegtl::string<'V', 'A', 'L', '_', 'T', 'A', 'B', 'L', 'E', '_'> {};
struct envvar_data_keyword : pegtl::string<'E', 'N', 'V', 'V', 'A', 'R', '_', 'D', 'A', 'T', 'A', '_'> {};

// Punctuation used in several parsers
struct colon : pegtl::one<':'> {};
struct comma : pegtl::one<','> {};
struct pipe : pegtl::one<'|'> {};
struct at_sign : pegtl::one<'@'> {};
struct lbracket : pegtl::one<'['> {};
struct rbracket : pegtl::one<']'> {};
struct lparen : pegtl::one<'('> {};
struct rparen : pegtl::one<')'> {};

}  // namespace common_grammar
}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_COMMON_GRAMMAR_H_ 