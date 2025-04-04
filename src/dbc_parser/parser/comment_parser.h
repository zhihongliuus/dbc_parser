#ifndef DBC_PARSER_PARSER_COMMENT_PARSER_H_
#define DBC_PARSER_PARSER_COMMENT_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace dbc_parser {
namespace parser {

// Enum to identify the type of object that a comment is associated with
enum class CommentType {
  NETWORK,        // Network comment (CM_)
  NODE,           // Node comment (CM_ BU_ <node name>)
  MESSAGE,        // Message comment (CM_ BO_ <message id>)
  SIGNAL,         // Signal comment (CM_ SG_ <message id> <signal name>)
  ENV_VAR         // Environment variable comment (CM_ EV_ <env var name>)
};

// Structure to hold comment data from DBC file
struct Comment {
  CommentType type;                   // Type of comment
  
  // Variant to hold the relevant identifiers based on comment type
  // - For NETWORK, no identifier is needed
  // - For NODE, the node name is stored
  // - For MESSAGE, the message id is stored
  // - For SIGNAL, both message id and signal name are stored
  // - For ENV_VAR, the environment variable name is stored
  std::variant<
    std::monostate,                  // NETWORK
    std::string,                     // NODE, ENV_VAR
    int,                             // MESSAGE
    std::pair<int, std::string>      // SIGNAL
  > identifier;
  
  std::string text;                  // The comment text
};

// Parser for CM_ entries in DBC files
class CommentParser {
 public:
  // Parses a comment string and returns a Comment object if parsing is successful
  // Returns std::nullopt if parsing fails
  //
  // Example formats:
  // CM_ "Network comment";
  // CM_ BU_ NodeName "Node comment";
  // CM_ BO_ 123 "Message comment";
  // CM_ SG_ 123 SignalName "Signal comment";
  // CM_ EV_ EnvVarName "Environment variable comment";
  static std::optional<Comment> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_COMMENT_PARSER_H_ 