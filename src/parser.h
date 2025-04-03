#ifndef SRC_PARSER_H_
#define SRC_PARSER_H_

#include <string>
#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>
#include "ast/dbc_ast.h"

namespace dbc_parser {

// Internal state used by PEGTL actions during parsing
struct ParsingState {
  // The final AST being built
  std::unique_ptr<ast::DbcFile> dbc_file = std::make_unique<ast::DbcFile>();
  
  // Context tracking
  std::stack<ast::Message*> message_stack; // Stack to keep track of the current message context

  // Temporary storage for assembling objects
  ast::Message current_message_temp; 
  ast::Signal current_signal_temp;
  ast::AttributeDefinition current_attr_def_temp;
  ast::SignalGroup current_sig_group_temp;
  ast::ValueTable current_val_table_temp; // For VAL_TABLE_

  // Temporary storage for multi-part rules (comments, attributes, values)
  std::string last_string; // Generic temporary string storage (e.g., for quoted strings)
  
  // Comment state
  std::string comment_type; // "NODE", "MESSAGE", "SIGNAL", "GLOBAL"
  std::string comment_target_node;
  uint32_t    comment_target_msg_id = 0;
  std::string comment_target_sig_name;
  std::string comment_text_content;

  // Attribute Definition state
  std::vector<std::string> enum_values_temp; 
  
  // Attribute Default state
  std::string attr_default_name_temp;
  std::string attr_default_value_temp;

  // Attribute Assignment state
  std::string attr_assign_name_temp;
  std::string attr_assign_type_temp = "GLOBAL";
  std::string attr_assign_target_node_temp;
  uint32_t    attr_assign_target_msg_id_temp = 0;
  std::string attr_assign_target_sig_name_temp;
  std::string attr_assign_value_temp;

  // Value Table / Value Description state
  std::unordered_map<int, std::string> val_desc_list_temp;
  int         val_desc_value_temp = 0;
  std::string val_desc_text_temp;
  // For VAL_ specific targets
  uint32_t    val_target_msg_id_temp = 0;
  std::string val_target_sig_name_temp;

  // Signal Group state
  std::vector<std::string> sig_group_members_temp;

  // Helper to get current message, returns nullptr if stack is empty
  ast::Message* GetCurrentMessage() {
      return message_stack.empty() ? nullptr : message_stack.top();
  }
  
  // Reset temporary fields associated with a specific section after processing
  void ResetCommentState() {
      comment_type = "";
      comment_target_node = "";
      comment_target_msg_id = 0;
      comment_target_sig_name = "";
      comment_text_content = "";
  }
  
  void ResetAttributeDefinitionState() {
      current_attr_def_temp = {}; // Reset temp object
      current_attr_def_temp.object_type = ""; // Default global
      enum_values_temp.clear();
  }
  
  void ResetAttributeDefaultState() {
       attr_default_name_temp = "";
       attr_default_value_temp = "";
  }
  
   void ResetAttributeAssignmentState() {
        attr_assign_name_temp = "";
        attr_assign_type_temp = "GLOBAL"; 
        attr_assign_target_node_temp = "";
        attr_assign_target_msg_id_temp = 0;
        attr_assign_target_sig_name_temp = "";
        attr_assign_value_temp = "";
   }
   
   void ResetValueDescriptionState() {
       val_desc_list_temp.clear();
       val_desc_value_temp = 0;
       val_desc_text_temp = "";
       val_target_msg_id_temp = 0;
       val_target_sig_name_temp = "";
       current_val_table_temp = {}; // Also reset val table temp
   }
   
   void ResetSignalGroupState() {
       current_sig_group_temp = {};
       sig_group_members_temp.clear();
   }
};

// Main parser class that handles DBC file parsing
class DbcParser {
 public:
  DbcParser() = default;
  ~DbcParser() = default;

  // Disable copy and move operations
  DbcParser(const DbcParser&) = delete;
  DbcParser& operator=(const DbcParser&) = delete;
  DbcParser(DbcParser&&) = delete;
  DbcParser& operator=(DbcParser&&) = delete;

  /**
   * @brief Parses a DBC file from the given file path.
   *
   * @param file_path Path to the DBC file.
   * @param dbc_file Output parameter, a unique pointer to the parsed DbcFile AST.
   *                 Will be reset if parsing fails.
   * @return True if parsing was successful, false otherwise.
   */
  bool Parse(const std::string& file_path, std::unique_ptr<ast::DbcFile>* dbc_file);

  /**
   * @brief Parses DBC content from a string.
   *
   * @param content The DBC content as a string.
   * @param source_name A name for the source (e.g., "string_input") used in error messages.
   * @param dbc_file Output parameter, a unique pointer to the parsed DbcFile AST.
   *                 Will be reset if parsing fails.
   * @return True if parsing was successful, false otherwise.
   */
  bool ParseString(const std::string& content, const std::string& source_name, std::unique_ptr<ast::DbcFile>* dbc_file);

  /**
   * @brief Get the last error message if parsing failed.
   *
   * @return A string containing the last error message.
   */
  std::string GetLastError() const;

 private:
  std::string last_error_;
  // Internal parsing state and helper methods might go here later
};

}  // namespace dbc_parser

#endif  // SRC_PARSER_H_ 