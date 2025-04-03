#ifndef SRC_GRAMMAR_DBC_ACTIONS_H_
#define SRC_GRAMMAR_DBC_ACTIONS_H_

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>
#include <tao/pegtl/contrib/unescape.hpp>

#include <iostream> // For temporary debug output
#include <string> 
#include <vector>
#include <unordered_map>

#include "../ast/dbc_ast.h"
#include "../parser.h" // Include parser to get ParsingState definition
#include "dbc_grammar.h"

namespace dbc_parser {

namespace grammar {

namespace pegtl = tao::pegtl;

// Helper to convert string to integer, with error handling
int string_to_int(const std::string& s, int default_value = 0) {
    try {
        return std::stoi(s);
    } catch (...) { 
        // Handle exceptions like invalid_argument, out_of_range
        return default_value;
    }
}

// Helper to convert string to double, with error handling
double string_to_double(const std::string& s, double default_value = 0.0) {
    try {
        return std::stod(s);
    } catch (...) {
        return default_value;
    }
}

// Base action template 
template< typename Rule >
struct action : pegtl::nothing< Rule > {};

// --- Action Specializations --- 

// Action for double_quoted_string_content - Stores the raw content into state.last_string
template<>
struct action< double_quoted_string_content > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        state.last_string = in.string(); // Store in state
        // std::cout << "Temp String: " << state.last_string << std::endl;
    }
};

// Action for VERSION section
template<>
struct action< version_section > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        // Assumes double_quoted_string_content action stored the content in state.last_string
        state.dbc_file->version = state.last_string;
        // TODO: Unescaping if needed: pegtl::unescape::unescape_c( state.last_string, state.dbc_file->version );
        std::cout << "Parsed Version: " << state.dbc_file->version << std::endl;
    }
};

// Action for individual node_name in BU_ section
template<>
struct action< node_name > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        // Add the node name directly
        state.dbc_file->nodes.push_back({in.string(), "", {}}); 
        std::cout << "Parsed Node: " << in.string() << std::endl;
    }
};

// --- Message Actions --- 

template<>
struct action< message_id > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        state.current_message_temp.id = string_to_int(in.string());
    }
};

template<>
struct action< message_name > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        state.current_message_temp.name = in.string();
    }
};

template<>
struct action< message_dlc > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        state.current_message_temp.dlc = string_to_int(in.string());
    }
};

template<>
struct action< message_sender > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        state.current_message_temp.sender = in.string();
    }
};

// Action when a full message header is matched
template<>
struct action< message_header > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        state.dbc_file->messages.push_back(state.current_message_temp);
        state.message_stack.push(&state.dbc_file->messages.back()); // Push current message onto stack
        state.current_message_temp = {}; // Reset temporary in state
        std::cout << "Started Message: " << state.message_stack.top()->name << " (" << state.message_stack.top()->id << ")" << std::endl;
    }
};

// --- Signal Actions --- 

template<>
struct action< signal_name > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        state.current_signal_temp.name = in.string();
    }
};

// TODO: Actions for multiplexer_indicator (need to handle M vs m<val>)

template<>
struct action< signal_start_bit > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        state.current_signal_temp.start_bit = string_to_int(in.string());
    }
};

template<>
struct action< signal_length > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        state.current_signal_temp.length = string_to_int(in.string());
    }
};

template<>
struct action< signal_byte_order > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        state.current_signal_temp.byte_order = (in.string() == "0") ? ast::ByteOrder::kBigEndian : ast::ByteOrder::kLittleEndian;
    }
};

template<>
struct action< signal_sign > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        state.current_signal_temp.is_signed = (in.string() == "-");
    }
};

template<>
struct action< signal_factor > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        state.current_signal_temp.factor = string_to_double(in.string());
    }
};

template<>
struct action< signal_offset > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        state.current_signal_temp.offset = string_to_double(in.string());
    }
};

template<>
struct action< signal_min > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        state.current_signal_temp.min_value = string_to_double(in.string());
    }
};

template<>
struct action< signal_max > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        state.current_signal_temp.max_value = string_to_double(in.string());
    }
};

template<>
struct action< signal_unit > { // Matches double_quoted_string rule
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        // Assumes double_quoted_string_content action stored content in state.last_string
        state.current_signal_temp.unit = state.last_string;
        // TODO: Add unescaping if needed
    }
};

template<>
struct action< receiver_node > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        state.current_signal_temp.receiver_nodes.push_back(in.string());
    }
};

// Action when a full signal section is matched
template<>
struct action< signal_section > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        ast::Message* current_msg = state.GetCurrentMessage();
        if (current_msg) {
            current_msg->signals.push_back(state.current_signal_temp);
            std::cout << "  Added Signal: " << state.current_signal_temp.name << " to Message: " << current_msg->name << std::endl;
        }
        state.current_signal_temp = {}; // Reset temporary in state
    }
};

// --- Comment Actions --- 

// Actions to capture comment type and targets
template<>
struct action< comment_type_node > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.comment_type = "NODE";
    }
};
template<>
struct action< comment_type_message > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.comment_type = "MESSAGE";
    }
};
template<>
struct action< comment_type_signal > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.comment_type = "SIGNAL";
    }
};

template<>
struct action< comment_node_target > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.comment_target_node = in.string();
    }
};
template<>
struct action< comment_message_target > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.comment_target_msg_id = string_to_int(in.string());
    }
};
template<>
struct action< comment_signal_target_msg_id > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.comment_target_msg_id = string_to_int(in.string());
    }
};
template<>
struct action< comment_signal_target_sig_name > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.comment_target_sig_name = in.string();
    }
};

// Reusing double_quoted_string_content action to store in state.last_string
// Need to refine this for comment context
template<>
struct action< comment_text > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        // Assumes double_quoted_string_content action stored content in state.last_string
        state.comment_text_content = state.last_string;
    }
};

// Action triggered when a complete comment section is parsed
template<>
struct action< comment_section > {
    template< typename Input >
    static void apply( const Input& in, ParsingState& state ) {
        std::string comment_text_final = state.comment_text_content; 
        // TODO: Add unescaping if necessary: pegtl::unescape::unescape_c(state.comment_text_content, comment_text_final);

        if (state.comment_type == "NODE") {
            ast::Node* node = state.dbc_file->FindNode(state.comment_target_node);
            if (node) {
                node->comment = comment_text_final;
                std::cout << "Applied Comment to Node: " << state.comment_target_node << std::endl;
            } // TODO: Handle node not found error
        } else if (state.comment_type == "MESSAGE") {
            ast::Message* msg = state.dbc_file->FindMessage(state.comment_target_msg_id);
            if (msg) {
                msg->comment = comment_text_final;
                std::cout << "Applied Comment to Message: " << state.comment_target_msg_id << std::endl;
            } // TODO: Handle message not found
        } else if (state.comment_type == "SIGNAL") {
            ast::Signal* sig = state.dbc_file->FindSignal(state.comment_target_msg_id, state.comment_target_sig_name);
            if (sig) {
                sig->comment = comment_text_final;
                std::cout << "Applied Comment to Signal: " << state.comment_target_msg_id << "/" << state.comment_target_sig_name << std::endl;
            } // TODO: Handle signal not found
        } else { // Global comment (type wasn't explicitly matched)
           state.comment_type = "GLOBAL"; // Explicitly mark as global if type sub-rules didn't match
           // TODO: Store global comment (needs a field in DbcFile AST)
           std::cout << "Found Global Comment: " << comment_text_final << std::endl;
        }

        state.ResetCommentState(); // Use helper to reset state fields
    }
};

// --- Attribute Definition Actions --- 

// Capture object type if present
template<>
struct action< attr_def_type_node > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.current_attr_def_temp.object_type = "BU_";
    }
};
template<>
struct action< attr_def_type_message > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.current_attr_def_temp.object_type = "BO_";
    }
};
template<>
struct action< attr_def_type_signal > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.current_attr_def_temp.object_type = "SG_";
    }
};

// Capture attribute name (content within quotes)
template<>
struct action< attr_def_name > { // Matches double_quoted_string
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        // Assumes double_quoted_string_content action stored content in state.last_string
        state.current_attr_def_temp.name = state.last_string;
        // Add unescaping if needed
    }
};

// Capture data type keywords
template<>
struct action< attr_type_int > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.current_attr_def_temp.type = "INT";
    }
};
template<>
struct action< attr_type_hex > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.current_attr_def_temp.type = "HEX";
    }
};
template<>
struct action< attr_type_float > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.current_attr_def_temp.type = "FLOAT";
    }
};
template<>
struct action< attr_type_string > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.current_attr_def_temp.type = "STRING";
    }
};
template<>
struct action< attr_type_enum > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.current_attr_def_temp.type = "ENUM";
        state.enum_values_temp.clear(); // Clear previous enum values when starting a new ENUM def
    }
};

// Capture min/max values
template<>
struct action< attr_def_num_min > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.current_attr_def_temp.min_value = string_to_double(in.string()); 
    }
};
template<>
struct action< attr_def_num_max > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.current_attr_def_temp.max_value = string_to_double(in.string());
    }
};
template<>
struct action< attr_def_float_min > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.current_attr_def_temp.min_value = string_to_double(in.string());
    }
};
template<>
struct action< attr_def_float_max > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.current_attr_def_temp.max_value = string_to_double(in.string());
    }
};

// Capture individual ENUM values (content within quotes)
template<>
struct action< attr_def_enum_val > { // Matches double_quoted_string
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        // Assumes double_quoted_string_content action stored content in state.last_string
        state.enum_values_temp.push_back(state.last_string);
        // Add unescaping if needed
    }
};

// Action when the full BA_DEF_ section is parsed
template<>
struct action< attribute_definition_section > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        if (state.current_attr_def_temp.type == "ENUM") {
            state.current_attr_def_temp.enum_values = std::move(state.enum_values_temp);
        }
        state.dbc_file->attribute_definitions.push_back(state.current_attr_def_temp);
        std::cout << "Added Attribute Definition: " << state.current_attr_def_temp.name 
                  << " Type: " << state.current_attr_def_temp.type 
                  << " Object: " << (state.current_attr_def_temp.object_type.empty() ? "GLOBAL" : state.current_attr_def_temp.object_type)
                  << std::endl;
        state.ResetAttributeDefinitionState(); // Use helper to reset
    }
};

// --- Attribute Default Definition Actions --- 

// Capture the name for BA_DEF_DEF_
template<>
struct action< attr_def_default_name > { // Matches double_quoted_string
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
         // Assumes double_quoted_string_content action stored content in state.last_string
        state.attr_default_name_temp = state.last_string;
        // Add unescaping if needed
    }
};

// Capture the default value for BA_DEF_DEF_
// Need separate actions for number vs string default values
template<>
struct action< attr_def_default_value > { 
     template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.attr_default_value_temp = in.string();
        // If it's a quoted string, strip quotes (handled by double_quoted_string_content)
        if (state.attr_default_value_temp.length() >= 2 && state.attr_default_value_temp.front() == '"' && state.attr_default_value_temp.back() == '"' ) {
             state.attr_default_value_temp = state.attr_default_value_temp.substr(1, state.attr_default_value_temp.length() - 2);
             // Add unescaping if needed
        }
    }
};

// Action when the full BA_DEF_DEF_ section is parsed
template<>
struct action< attribute_default_definition_section > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        ast::AttributeDefinition* attr_def = state.dbc_file->FindAttributeDefinition(state.attr_default_name_temp);
        if (attr_def) {
            attr_def->default_value = state.attr_default_value_temp;
            std::cout << "Set Default Value for Attribute: " << state.attr_default_name_temp 
                      << " to: " << state.attr_default_value_temp << std::endl;
        } else {
             std::cerr << "Error: BA_DEF_DEF_ for undefined attribute '" << state.attr_default_name_temp << "'" << std::endl;
        }
        state.ResetAttributeDefaultState(); // Use helper to reset
    }
};

// --- Attribute Assignment Actions (BA_) --- 

// Capture attribute name reference
template<>
struct action< attribute_name_ref > { // Matches double_quoted_string
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        // Assumes double_quoted_string_content action stored content in state.last_string
        state.attr_assign_name_temp = state.last_string;
        // Add unescaping if needed
    }
};

// Capture target types
template<>
struct action< attr_target_type_node > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.attr_assign_type_temp = "NODE";
    }
};
template<>
struct action< attr_target_type_message > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.attr_assign_type_temp = "MESSAGE";
    }
};
template<>
struct action< attr_target_type_signal > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.attr_assign_type_temp = "SIGNAL";
    }
};

// Capture target identifiers
template<>
struct action< attr_node_target > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.attr_assign_target_node_temp = in.string();
    }
};
template<>
struct action< attr_message_target > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.attr_assign_target_msg_id_temp = string_to_int(in.string());
    }
};
template<>
struct action< attr_signal_target_msg_id > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.attr_assign_target_msg_id_temp = string_to_int(in.string());
    }
};
template<>
struct action< attr_signal_target_sig_name > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.attr_assign_target_sig_name_temp = in.string();
    }
};

// Capture attribute value
template<>
struct action< attribute_value > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.attr_assign_value_temp = in.string();
        // Strip quotes if it's a string value
        if (state.attr_assign_value_temp.length() >= 2 && state.attr_assign_value_temp.front() == '"' && state.attr_assign_value_temp.back() == '"' ) {
             state.attr_assign_value_temp = state.attr_assign_value_temp.substr(1, state.attr_assign_value_temp.length() - 2);
             // Add unescaping if needed
        }
    }
};

// Action when the full BA_ section is parsed
template<>
struct action< attribute_assignment_section > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        std::string& attr_name = state.attr_assign_name_temp;
        std::string& attr_value = state.attr_assign_value_temp;
        std::string& target_type = state.attr_assign_type_temp;
        
        if (target_type == "NODE") {
            ast::Node* node = state.dbc_file->FindNode(state.attr_assign_target_node_temp);
            if (node) {
                node->attributes[attr_name] = attr_value;
                std::cout << "Assigned Attribute '" << attr_name << "'='" << attr_value << "' to Node: " << state.attr_assign_target_node_temp << std::endl;
            } // TODO: Handle error
        } else if (target_type == "MESSAGE") {
            ast::Message* msg = state.dbc_file->FindMessage(state.attr_assign_target_msg_id_temp);
            if (msg) {
                msg->attributes[attr_name] = attr_value;
                std::cout << "Assigned Attribute '" << attr_name << "'='" << attr_value << "' to Message: " << state.attr_assign_target_msg_id_temp << std::endl;
            } // TODO: Handle error
        } else if (target_type == "SIGNAL") {
            ast::Signal* sig = state.dbc_file->FindSignal(state.attr_assign_target_msg_id_temp, state.attr_assign_target_sig_name_temp);
            if (sig) {
                sig->attributes[attr_name] = attr_value;
                 std::cout << "Assigned Attribute '" << attr_name << "'='" << attr_value << "' to Signal: " 
                           << state.attr_assign_target_msg_id_temp << "/" << state.attr_assign_target_sig_name_temp << std::endl;
            } // TODO: Handle error
        } else { // GLOBAL
            // TODO: Store global attribute assignment (needs structure in DbcFile AST)
            std::cout << "Assigned Global Attribute '" << attr_name << "'='" << attr_value << "'" << std::endl;
        }
        state.ResetAttributeAssignmentState(); // Use helper to reset
    }
};

// --- Value Table / Value Description Actions --- 

// Capture value table name
template<>
struct action< value_table_name > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.current_val_table_temp.name = in.string();
        state.val_desc_list_temp.clear(); // Clear list for new table
    }
};

// Capture target message/signal for VAL_
template<>
struct action< val_target_msg_id > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.val_target_msg_id_temp = string_to_int(in.string());
        state.val_desc_list_temp.clear(); // Clear list for new VAL_ entry
    }
};
template<>
struct action< val_target_sig_name > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.val_target_sig_name_temp = in.string();
    }
};

// Capture individual value/description pair parts
template<>
struct action< value_description_value > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.val_desc_value_temp = string_to_int(in.string());
    }
};

template<>
struct action< value_description_text > { // Matches double_quoted_string
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
         // Assumes double_quoted_string_content action stored content in state.last_string
        state.val_desc_text_temp = state.last_string;
        // Add unescaping if needed
    }
};

// Action when a full value description pair is parsed
template<>
struct action< value_description_pair > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.val_desc_list_temp[state.val_desc_value_temp] = state.val_desc_text_temp;
        // Reset individual pair temps (optional)
        state.val_desc_value_temp = 0;
        state.val_desc_text_temp = "";
    }
};

// Action when a full VAL_TABLE_ section is parsed
template<>
struct action< value_table_section > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        ast::ValueTable table = state.current_val_table_temp; // Copy name captured earlier
        table.value_descriptions = std::move(state.val_desc_list_temp); 
        state.dbc_file->value_tables.push_back(table);
        std::cout << "Added Value Table: " << table.name << " with " << table.value_descriptions.size() << " entries" << std::endl;
        state.ResetValueDescriptionState(); // Reset associated state
    }
};

// Action when a full VAL_ section is parsed
template<>
struct action< value_description_section > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        ast::Signal* sig = state.dbc_file->FindSignal(state.val_target_msg_id_temp, state.val_target_sig_name_temp);
        if (sig) {
            sig->value_descriptions = std::move(state.val_desc_list_temp);
             std::cout << "Added Value Descriptions to Signal: " << state.val_target_msg_id_temp << "/" << state.val_target_sig_name_temp 
                       << " (" << sig->value_descriptions.size() << " entries)" << std::endl;
        } else {
            std::cerr << "Error: VAL_ for undefined signal '" << state.val_target_msg_id_temp << "/" << state.val_target_sig_name_temp << "'" << std::endl;
        }
        state.ResetValueDescriptionState(); // Reset associated state
    }
};

// --- Signal Group Actions --- 

// Capture target message ID
template<>
struct action< sig_group_target_msg_id > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.current_sig_group_temp.message_id = string_to_int(in.string());
        state.sig_group_members_temp.clear(); // Clear members for new group
    }
};

// Capture group name
template<>
struct action< sig_group_name > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.current_sig_group_temp.name = in.string();
    }
};

// Capture repetitions
template<>
struct action< sig_group_repetitions > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.current_sig_group_temp.repetitions = string_to_int(in.string());
    }
};

// Capture individual signal names in the list
template<>
struct action< sig_group_member_sig_name > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.sig_group_members_temp.push_back(in.string());
    }
};

// Action when the full SIG_GROUP_ section is parsed
template<>
struct action< signal_group_section > {
    template< typename Input > static void apply( const Input& in, ParsingState& state ) {
        state.current_sig_group_temp.signal_names = std::move(state.sig_group_members_temp);
        state.dbc_file->signal_groups.push_back(state.current_sig_group_temp);
        std::cout << "Added Signal Group: " << state.current_sig_group_temp.name 
                  << " for Message: " << state.current_sig_group_temp.message_id 
                  << " with " << state.current_sig_group_temp.signal_names.size() << " members" 
                  << std::endl;
        state.ResetSignalGroupState(); // Use helper to reset
    }
};

} // namespace grammar
} // namespace dbc_parser

#endif // SRC_GRAMMAR_DBC_ACTIONS_H_ 