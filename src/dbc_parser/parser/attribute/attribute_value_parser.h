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

/**
 * @brief Represents an attribute value assignment in a DBC file.
 *
 * Attribute values associate specific values with attributes for particular
 * objects in the DBC file. These assignments override the default values
 * defined in attribute definition defaults.
 */
struct AttributeValue {
  std::string name;                  ///< Name of the attribute
  AttributeObjectType object_type;   ///< Object type this attribute applies to (network, node, message, signal, etc.)
  
  /**
   * @brief Object identifier (depends on object_type)
   * 
   * This variant holds different types of identifiers depending on the object type:
   * - NETWORK: std::monostate (no identifier needed)
   * - NODE: std::string (node name)
   * - ENV_VAR: std::string (environment variable name)
   * - MESSAGE: int (message ID)
   * - SIGNAL: std::pair<int, std::string> (message_id, signal_name)
   */
  std::variant<
    std::monostate,                  ///< NETWORK: no identifier
    std::string,                     ///< NODE: node name, ENV_VAR: env var name
    int,                             ///< MESSAGE: message ID
    std::pair<int, std::string>      ///< SIGNAL: (message_id, signal_name)
  > object_id;
  
  std::variant<int, double, std::string> value; ///< Attribute value (type depends on attribute definition)
};

/**
 * @brief Parser for attribute value assignments (BA_) in DBC files.
 *
 * Handles parsing of attribute value entries, which assign specific values
 * to attributes for particular objects in the DBC file. These entries allow
 * for customizing properties of various elements beyond the standard DBC format.
 *
 * Example DBC attribute value formats:
 * - BA_ "AttributeName" 42;                      (Network attribute)
 * - BA_ "AttributeName" BU_ "NodeName" 42;       (Node attribute)
 * - BA_ "AttributeName" BO_ 123 42;              (Message attribute)
 * - BA_ "AttributeName" SG_ 123 "SignalName" 42; (Signal attribute)
 * - BA_ "AttributeName" EV_ "EnvVarName" 42;     (Environment variable attribute)
 */
class AttributeValueParser : public ParserBase {
 public:
  /**
   * @brief Parses an attribute value assignment from the given input string.
   *
   * Takes a string containing a DBC BA_ entry and parses it into an
   * AttributeValue object. The parser validates the syntax and extracts
   * the attribute name, object type, object identifier, and assigned value.
   *
   * @param input String view containing the attribute value assignment to parse
   * @return std::optional<AttributeValue> An AttributeValue object if parsing succeeds, std::nullopt otherwise
   */
  [[nodiscard]] static std::optional<AttributeValue> Parse(std::string_view input);
};

} // namespace parser
} // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_ATTRIBUTE_VALUE_PARSER_H_ 