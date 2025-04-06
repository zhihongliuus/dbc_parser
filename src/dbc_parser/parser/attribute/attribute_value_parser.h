#ifndef DBC_PARSER_PARSER_ATTRIBUTE_VALUE_PARSER_H_
#define DBC_PARSER_PARSER_ATTRIBUTE_VALUE_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "dbc_parser/parser/attribute/attribute_definition_parser.h"
#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

// Structure to hold an attribute value
struct AttributeValue {
  std::string name;                  // Name of the attribute
  AttributeObjectType object_type;   // Object type this attribute applies to
  
  // Object identifier (depends on object_type)
  std::variant<
    std::monostate,                  // NETWORK: no identifier
    std::string,                     // NODE: node name, ENV_VAR: env var name
    int,                             // MESSAGE: message ID
    std::pair<int, std::string>      // SIGNAL: (message_id, signal_name)
  > object_id;
  
  // Attribute value
  std::variant<int, double, std::string> value;
};

// Parser for BA_ entries in DBC files
class AttributeValueParser : public ParserBase {
 public:
  // Parses an attribute value string and returns an AttributeValue object if parsing is successful
  // Returns std::nullopt if parsing fails
  //
  // Example formats:
  // BA_ "AttributeName" 42; (Network attribute)
  // BA_ "AttributeName" BU_ "NodeName" 42; (Node attribute)
  // BA_ "AttributeName" BO_ 123 42; (Message attribute)
  // BA_ "AttributeName" SG_ 123 "SignalName" 42; (Signal attribute)
  // BA_ "AttributeName" EV_ "EnvVarName" 42; (Environment variable attribute)
  static std::optional<AttributeValue> Parse(std::string_view input);
};

} // namespace parser
} // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_ATTRIBUTE_VALUE_PARSER_H_ 