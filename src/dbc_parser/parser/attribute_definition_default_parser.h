#ifndef DBC_PARSER_PARSER_ATTRIBUTE_DEFINITION_DEFAULT_PARSER_H_
#define DBC_PARSER_PARSER_ATTRIBUTE_DEFINITION_DEFAULT_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "src/dbc_parser/parser/attribute_definition_parser.h"
#include "src/dbc_parser/parser/parser_base.h"

namespace dbc_parser {
namespace parser {

// Structure to hold attribute definition default value
struct AttributeDefinitionDefault {
  std::string name;                  // Name of the attribute
  AttributeValueType value_type;     // Type of the attribute value
  
  // Default value for the attribute
  std::variant<int, double, std::string> default_value;
};

// Parser for BA_DEF_DEF_ entries in DBC files
class AttributeDefinitionDefaultParser : public ParserBase {
 public:
  // Parses an attribute definition default string and returns an AttributeDefinitionDefault object if parsing is successful
  // Returns std::nullopt if parsing fails
  //
  // Example formats:
  // BA_DEF_DEF_ "IntAttribute" 0; (Integer default)
  // BA_DEF_DEF_ "FloatAttribute" 0.5; (Float default)
  // BA_DEF_DEF_ "StringAttribute" "Default"; (String default)
  // BA_DEF_DEF_ "EnumAttribute" 1; (Enum default as index)
  static std::optional<AttributeDefinitionDefault> Parse(std::string_view input);
};

} // namespace parser
} // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_ATTRIBUTE_DEFINITION_DEFAULT_PARSER_H_ 