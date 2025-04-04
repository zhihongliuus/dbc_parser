#ifndef DBC_PARSER_PARSER_ATTRIBUTE_DEFINITION_PARSER_H_
#define DBC_PARSER_PARSER_ATTRIBUTE_DEFINITION_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <variant>

namespace dbc_parser {
namespace parser {

// The object type to which an attribute applies
enum class AttributeObjectType {
  UNDEFINED,
  NETWORK,  // Global attribute for the entire network
  NODE,     // Applies to a node (BU_)
  MESSAGE,  // Applies to a message (BO_)
  SIGNAL,   // Applies to a signal (SG_)
  ENV_VAR   // Applies to an environment variable (EV_)
};

// Type of attribute value
enum class AttributeValueType {
  INT,      // Integer value
  HEX,      // Hexadecimal value
  FLOAT,    // Floating point value
  STRING,   // String value
  ENUM      // Enumeration value
};

// Structure to hold attribute definition data
struct AttributeDefinition {
  std::string name;                  // Name of the attribute
  AttributeObjectType object_type;   // Object type this attribute applies to
  AttributeValueType value_type;     // Type of the attribute value
  
  // Optional minimum/maximum values for numeric types
  std::optional<double> min_value;
  std::optional<double> max_value;
  
  // Optional enumeration values for ENUM type
  std::vector<std::string> enum_values;
  
  // Default value for the attribute
  std::variant<int, double, std::string> default_value;
};

// Parser for BA_DEF_ entries in DBC files
class AttributeDefinitionParser {
 public:
  // Parses an attribute definition string and returns an AttributeDefinition object if parsing is successful
  // Returns std::nullopt if parsing fails
  //
  // Example formats:
  // BA_DEF_ "AttributeName" INT 0 100; (Integer attribute with range)
  // BA_DEF_ SG_ "AttributeName" FLOAT 0 1; (Signal floating point attribute with range)
  // BA_DEF_ BO_ "AttributeName" STRING; (Message string attribute)
  // BA_DEF_ "AttributeName" ENUM "Val1","Val2","Val3"; (Enumeration attribute)
  static std::optional<AttributeDefinition> Parse(std::string_view input);
};

} // namespace parser
} // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_ATTRIBUTE_DEFINITION_PARSER_H_ 