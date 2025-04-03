#ifndef DBC_PARSER_SRC_GRAMMAR_DBC_GRAMMAR_H_
#define DBC_PARSER_SRC_GRAMMAR_DBC_GRAMMAR_H_

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>

#include <string>
#include <vector>

namespace dbc_parser {
namespace grammar {

namespace pegtl = tao::pegtl;

// --- Basic Rules ---

// Whitespace and skipping
struct ws : pegtl::plus< pegtl::space > {}; // One or more spaces
struct opt_ws : pegtl::star< pegtl::space > {}; // Zero or more spaces
struct req_ws : ws {}; // Required whitespace (one or more)
struct skipper : opt_ws {}; // Optional whitespace for skipping

// Basic character classes
struct digit : pegtl::digit {};
struct alpha : pegtl::alpha {};
struct alphanum : pegtl::alnum {};
struct hex_digit : pegtl::xdigit {};

// Numbers
struct unsigned_integer : pegtl::plus< digit > {};
struct signed_integer : pegtl::seq< pegtl::opt< pegtl::one<'+', '-'> >, unsigned_integer > {};
struct float_num : pegtl::seq< signed_integer, pegtl::one<'.'>, unsigned_integer > {};

// Identifiers
struct identifier : pegtl::seq< 
    pegtl::sor< alpha, pegtl::one<'_'> >,
    pegtl::star< pegtl::sor< alphanum, pegtl::one<'_'> > >
> {};

// String handling
struct escaped_char : pegtl::seq< pegtl::one<'\\'>, pegtl::any > {};
struct regular_char : pegtl::not_one<'"', '\\'> {};
struct string_char : pegtl::sor< regular_char, escaped_char > {};
struct double_quoted_string_content : pegtl::star< string_char > {};
struct double_quoted_string : pegtl::if_must< 
    pegtl::one<'"'>, 
    double_quoted_string_content,
    pegtl::one<'"'> 
> {};

// Basic message components
struct message_id : signed_integer {};
struct node_name : identifier {};
struct signal_name : identifier {};

// Version section
struct version_keyword : pegtl::string<'V', 'E', 'R', 'S', 'I', 'O', 'N', '_'> {};
struct version_string : double_quoted_string {};
struct version_section : pegtl::if_must< version_keyword, req_ws, version_string > {};

// Nodes section (BU_)
struct nodes_keyword : pegtl::string<'B', 'U', '_'> {};
struct node_list : pegtl::list< node_name, req_ws > {};
struct nodes_section : pegtl::if_must< nodes_keyword, req_ws, node_list > {};

// Message section (BO_)
struct message_keyword : pegtl::string<'B', 'O', '_'> {};
struct message_name : identifier {};
struct message_dlc : unsigned_integer {};
struct message_sender : node_name {};
struct message_header : pegtl::seq<
    message_keyword, req_ws,
    message_id, req_ws,
    message_name, req_ws,
    message_dlc, req_ws,
    message_sender
> {};
struct message_section : message_header {};

// Signal section (SG_)
struct signal_keyword : pegtl::string<'S', 'G', '_'> {};
struct signal_start_bit : unsigned_integer {};
struct signal_length : unsigned_integer {};
struct signal_byte_order : pegtl::one<'0', '1'> {}; // 0=Motorola (Big Endian), 1=Intel (Little Endian)
struct signal_sign : pegtl::one<'+', '-'> {}; // + for unsigned, - for signed
struct signal_factor : float_num {};
struct signal_offset : float_num {};
struct signal_min : float_num {};
struct signal_max : float_num {};
struct signal_unit : double_quoted_string {};
struct receiver_node : node_name {};
struct receiver_list : pegtl::list< receiver_node, pegtl::seq<ws, pegtl::one<','>, ws> > {};

struct signal_section : pegtl::if_must<
    signal_keyword, req_ws,
    signal_name, req_ws,
    pegtl::one<':'>, req_ws,
    signal_start_bit, req_ws,
    pegtl::one<'|'>, req_ws,
    signal_length, req_ws,
    pegtl::one<'@'>, req_ws,
    signal_byte_order, req_ws,
    signal_sign, req_ws,
    pegtl::one<'('>, req_ws,
    signal_factor, req_ws,
    pegtl::one<','>, req_ws,
    signal_offset, req_ws,
    pegtl::one<')'>, req_ws,
    pegtl::one<'['>, req_ws,
    signal_min, req_ws,
    pegtl::one<'|'>, req_ws,
    signal_max, req_ws,
    pegtl::one<']'>, req_ws,
    signal_unit, req_ws,
    receiver_list
> {};

// Simple placeholder for future grammar implementation
class DbcGrammar {
 public:
  DbcGrammar() = default;
  ~DbcGrammar() = default;

  // Parse a DBC file content
  bool Parse(const std::string& content);
};

// --- Specific DBC Section Rules --- 

// CM_ [BU_|BO_|SG_] [node_name|msg_id] [signal_name] "comment_string";
struct comment_keyword : pegtl::string<'C', 'M', '_'> {};
// Define specific comment types
struct comment_type_global : pegtl::success {}; // Implicit, just CM_ "string";
struct comment_type_node : pegtl::string<'B', 'U', '_'> {};
struct comment_type_message : pegtl::string<'B', 'O', '_'> {};
struct comment_type_signal : pegtl::string<'S', 'G', '_'> {};

// Object identifiers for comments
struct comment_node_target : node_name {};
struct comment_message_target : message_id {};
struct comment_signal_target_msg_id : message_id {};
struct comment_signal_target_sig_name : signal_name {};

struct comment_text : double_quoted_string {};

// Rule structure: CM_ [type target+] text
struct comment_section : pegtl::if_must<
    comment_keyword, req_ws,
    pegtl::sor<
        // Node comment: CM_ BU_ node_name "text"
        pegtl::seq< comment_type_node, req_ws, comment_node_target, req_ws, comment_text >,
        // Message comment: CM_ BO_ msg_id "text"
        pegtl::seq< comment_type_message, req_ws, comment_message_target, req_ws, comment_text >,
        // Signal comment: CM_ SG_ msg_id signal_name "text"
        pegtl::seq< comment_type_signal, req_ws, comment_signal_target_msg_id, req_ws, comment_signal_target_sig_name, req_ws, comment_text >,
        // Global comment: CM_ "text"
        comment_text 
    >
> {};

// BA_DEF_ [BU_|BO_|SG_] "attribute_name" type [min max | "val" ...];
// BA_DEF_ "attribute_name" type [min max | "val" ...]; // Global attribute
struct attr_def_keyword : pegtl::string<'B', 'A', '_', 'D', 'E', 'F', '_'> {};
struct attr_def_type_node : pegtl::string<'B', 'U', '_'> {};
struct attr_def_type_message : pegtl::string<'B', 'O', '_'> {};
struct attr_def_type_signal : pegtl::string<'S', 'G', '_'> {};
struct attr_def_object_type : pegtl::sor< attr_def_type_node, attr_def_type_message, attr_def_type_signal > {};

struct attr_def_name : double_quoted_string {};

// Attribute types
struct attr_type_int : pegtl::string<'I', 'N', 'T'> {};
struct attr_type_hex : pegtl::string<'H', 'E', 'X'> {};
struct attr_type_float : pegtl::string<'F', 'L', 'O', 'A', 'T'> {};
struct attr_type_string : pegtl::string<'S', 'T', 'R', 'I', 'N', 'G'> {};
struct attr_type_enum : pegtl::string<'E', 'N', 'U', 'M'> {};
struct attr_def_data_type : pegtl::sor< attr_type_int, attr_type_hex, attr_type_float, attr_type_string, attr_type_enum > {};

// Type-specific definitions
struct attr_def_num_min : signed_integer {}; // INT, HEX, FLOAT can have min/max
struct attr_def_num_max : signed_integer {}; 
struct attr_def_float_min : float_num {};
struct attr_def_float_max : float_num {};
struct attr_def_enum_val : double_quoted_string {};
struct attr_def_enum_list : pegtl::list< attr_def_enum_val, pegtl::seq<ws, pegtl::one<','>, ws> > {}; // Comma-separated quoted strings
struct attr_def_string_default : double_quoted_string {}; // STRING has optional default

// Combine type definitions
struct attr_def_type_details : pegtl::sor<
    pegtl::seq< pegtl::sor<attr_type_int, attr_type_hex>, req_ws, attr_def_num_min, req_ws, attr_def_num_max >, // INT/HEX min max
    pegtl::seq< attr_type_float, req_ws, attr_def_float_min, req_ws, attr_def_float_max >, // FLOAT min max
    pegtl::seq< attr_type_enum, req_ws, attr_def_enum_list >, // ENUM list
    attr_type_string // STRING has no following params in this part
> {};

// Main attribute definition rule
struct attribute_definition_section : pegtl::if_must<
    attr_def_keyword, req_ws,
    pegtl::opt< pegtl::seq< attr_def_object_type, req_ws > >, // Optional object type
    attr_def_name, req_ws,
    attr_def_type_details // Includes type keyword and its specific params
    // Optional default value for STRING/INT/HEX/FLOAT is handled by BA_DEF_DEF_ below
> {};

// BA_DEF_DEF_ "attribute_name" default_value;
struct attr_def_default_keyword : pegtl::string<'B', 'A', '_', 'D', 'E', 'F', '_', 'D', 'E', 'F', '_'> {};
struct attr_def_default_name : double_quoted_string {};
struct attr_def_default_value : pegtl::sor< float_num, signed_integer, double_quoted_string > {}; // Default can be number or string
struct attribute_default_definition_section : pegtl::if_must<
    attr_def_default_keyword, req_ws,
    attr_def_default_name, req_ws,
    attr_def_default_value
> {};

// BA_ "attribute_name" [BU_|BO_|SG_] [node_name|msg_id] [signal_name] value;
// BA_ "attribute_name" value; // Global attribute assignment
struct attribute_keyword : pegtl::string<'B', 'A', '_'> {};
struct attribute_name_ref : double_quoted_string {}; // Reference attribute name

// Object identifiers for attribute assignment (similar to comments)
struct attr_node_target : node_name {};
struct attr_message_target : message_id {};
struct attr_signal_target_msg_id : message_id {};
struct attr_signal_target_sig_name : signal_name {};

// Type of object attribute is applied to (same as BA_DEF_)
struct attr_target_type_node : attr_def_type_node {};
struct attr_target_type_message : attr_def_type_message {};
struct attr_target_type_signal : attr_def_type_signal {};
struct attr_target_object_type : attr_def_object_type {};

// Attribute value can be number or string
struct attribute_value : pegtl::sor< float_num, signed_integer, double_quoted_string > {};

// Rule structure: BA_ "name" [type target+] value
struct attribute_assignment_section : pegtl::if_must<
    attribute_keyword, req_ws,
    attribute_name_ref, req_ws,
    pegtl::sor<
        // Node attribute: BA_ "name" BU_ node_name value
        pegtl::seq< attr_target_type_node, req_ws, attr_node_target, req_ws, attribute_value >,
        // Message attribute: BA_ "name" BO_ msg_id value
        pegtl::seq< attr_target_type_message, req_ws, attr_message_target, req_ws, attribute_value >,
        // Signal attribute: BA_ "name" SG_ msg_id signal_name value
        pegtl::seq< attr_target_type_signal, req_ws, attr_signal_target_msg_id, req_ws, attr_signal_target_sig_name, req_ws, attribute_value >,
        // Global attribute: BA_ "name" value
        attribute_value
    >
> {};

// VAL_TABLE_ table_name val_desc_pair [val_desc_pair ...];
struct val_table_keyword : pegtl::string<'V', 'A', 'L', '_', 'T', 'A', 'B', 'L', 'E', '_'> {};
struct value_table_name : identifier {};
struct value_description_value : signed_integer {}; // Values can be signed
struct value_description_text : double_quoted_string {};
struct value_description_pair : pegtl::seq< value_description_value, req_ws, value_description_text > {};
struct value_description_list : pegtl::plus< pegtl::seq< value_description_pair, ws > > {}; // One or more pairs
struct value_table_section : pegtl::if_must<
    val_table_keyword, req_ws,
    value_table_name, req_ws,
    value_description_list
> {};

// VAL_ msg_id signal_name val_desc_pair [val_desc_pair ...];
struct val_keyword : pegtl::string<'V', 'A', 'L', '_'> {};
struct val_target_msg_id : message_id {};
struct val_target_sig_name : signal_name {};
struct value_description_section : pegtl::if_must<
    val_keyword, req_ws,
    val_target_msg_id, req_ws,
    val_target_sig_name, req_ws,
    value_description_list
> {};

// SIG_GROUP_ msg_id group_name repetitions : signal_name [signal_name ...];
struct sig_group_keyword : pegtl::string<'S', 'I', 'G', '_', 'G', 'R', 'O', 'U', 'P', '_'> {};
struct sig_group_target_msg_id : message_id {};
struct sig_group_name : identifier {};
struct sig_group_repetitions : unsigned_integer {};
struct sig_group_member_sig_name : signal_name {};
struct sig_group_signal_list : pegtl::list< sig_group_member_sig_name, req_ws > {}; // space-separated signal names
struct signal_group_section : pegtl::if_must<
    sig_group_keyword, req_ws,
    sig_group_target_msg_id, req_ws,
    sig_group_name, req_ws,
    sig_group_repetitions, ws, pegtl::one<':'>, ws,
    sig_group_signal_list
> {};

// --- Top-Level Grammar Rule --- 

struct dbc_section : pegtl::sor<
    version_section,
    nodes_section,
    message_section,
    comment_section,
    attribute_definition_section, 
    attribute_default_definition_section, 
    attribute_assignment_section,
    value_table_section, 
    value_description_section,
    signal_group_section // Added SIG_GROUP_
> {};

// The main rule: zero or more sections, separated by skipper, until EOF
// Now includes all major sections we've defined
struct dbc_file : pegtl::until< pegtl::eof, pegtl::seq< skipper, dbc_section > > {};

}  // namespace grammar
}  // namespace dbc_parser

#endif  // DBC_PARSER_SRC_GRAMMAR_DBC_GRAMMAR_H_ 