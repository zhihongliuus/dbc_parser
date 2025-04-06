#ifndef DBC_PARSER_PARSER_ATTRIBUTE_DEFINITION_PARSER_H_
#define DBC_PARSER_PARSER_ATTRIBUTE_DEFINITION_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <variant>

#include "dbc_parser/common/common_types.h"
#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

/**
 * @brief Represents an attribute definition from a DBC file.
 *
 * Attribute definitions specify metadata that can be associated with different
 * DBC objects (networks, messages, signals, etc.). They define the attribute's
 * name, applicable object type, value type, and constraints.
 */
struct AttributeDefinition {
  std::string name;                  ///< Name of the attribute
  AttributeObjectType object_type;   ///< Object type this attribute applies to (e.g., network, message, signal)
  AttributeValueType value_type;     ///< Type of the attribute value (e.g., integer, float, string, enum)
  
  std::optional<double> min_value;   ///< Optional minimum value for numeric types
  std::optional<double> max_value;   ///< Optional maximum value for numeric types
  
  std::vector<std::string> enum_values; ///< Optional enumeration values for ENUM type
  
  std::variant<int, double, std::string> default_value; ///< Default value for the attribute
};

/**
 * @brief Parser for attribute definitions (BA_DEF_) in DBC files.
 *
 * Handles parsing of attribute definition entries, which define metadata
 * attributes that can be associated with various elements in the DBC file.
 * Attributes provide additional information beyond the standard DBC format.
 *
 * Example DBC attribute definition formats:
 * - BA_DEF_ "AttributeName" INT 0 100;                  (Network integer attribute with range)
 * - BA_DEF_ SG_ "AttributeName" FLOAT 0 1;              (Signal floating point attribute with range)
 * - BA_DEF_ BO_ "AttributeName" STRING;                 (Message string attribute)
 * - BA_DEF_ "AttributeName" ENUM "Val1","Val2","Val3";  (Network enumeration attribute)
 *
 * The object type prefix (e.g., SG_, BO_) indicates which DBC elements the attribute
 * applies to. If no prefix is given, the attribute applies to the entire network.
 */
class AttributeDefinitionParser : public ParserBase {
 public:
  /**
   * @brief Parses an attribute definition from the given input string.
   *
   * Takes a string containing a DBC BA_DEF_ entry and parses it into an
   * AttributeDefinition object. The parser validates the syntax and extracts
   * the attribute name, object type, value type, and any constraints or options.
   *
   * @param input String view containing the attribute definition to parse
   * @return std::optional<AttributeDefinition> An AttributeDefinition object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<AttributeDefinition> Parse(std::string_view input);
};

} // namespace parser
} // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_ATTRIBUTE_DEFINITION_PARSER_H_ 