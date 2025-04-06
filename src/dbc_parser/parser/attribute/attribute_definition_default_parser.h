#ifndef DBC_PARSER_PARSER_ATTRIBUTE_DEFINITION_DEFAULT_PARSER_H_
#define DBC_PARSER_PARSER_ATTRIBUTE_DEFINITION_DEFAULT_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "attribute_definition_parser.h"
#include "dbc_parser/common/parser_base.h"

namespace dbc_parser {
namespace parser {

/**
 * @brief Represents the default value for an attribute definition in a DBC file.
 *
 * Attribute definition defaults specify the default values for attributes
 * when they are not explicitly assigned to objects. These are defined in the 
 * BA_DEF_DEF_ sections of DBC files.
 */
struct AttributeDefinitionDefault {
  std::string name;                  ///< Name of the attribute
  AttributeValueType value_type;     ///< Type of the attribute value (integer, float, string, enum)
  
  std::variant<int, double, std::string> default_value; ///< Default value for the attribute
};

/**
 * @brief Parser for attribute definition defaults (BA_DEF_DEF_) in DBC files.
 *
 * Handles parsing of attribute definition default entries, which define the
 * default values for attributes when they are not explicitly assigned to objects.
 * These entries complement the attribute definitions (BA_DEF_).
 *
 * Example DBC attribute definition default formats:
 * - BA_DEF_DEF_ "IntAttribute" 0;         (Integer default)
 * - BA_DEF_DEF_ "FloatAttribute" 0.5;     (Float default)
 * - BA_DEF_DEF_ "StringAttribute" "Default"; (String default)
 * - BA_DEF_DEF_ "EnumAttribute" 1;        (Enum default as index)
 */
class AttributeDefinitionDefaultParser : public ParserBase {
 public:
  /**
   * @brief Parses an attribute definition default from the given input string.
   *
   * Takes a string containing a DBC BA_DEF_DEF_ entry and parses it into an
   * AttributeDefinitionDefault object. The parser validates the syntax and extracts
   * the attribute name and default value.
   *
   * @param input String view containing the attribute definition default to parse
   * @return std::optional<AttributeDefinitionDefault> An AttributeDefinitionDefault object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<AttributeDefinitionDefault> Parse(std::string_view input);
};

} // namespace parser
} // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_ATTRIBUTE_DEFINITION_DEFAULT_PARSER_H_ 