#ifndef DBC_PARSER_DBC_GRAMMAR_H_
#define DBC_PARSER_DBC_GRAMMAR_H_

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/optional.hpp>

#include "dbc_parser/types.h"
#include "dbc_parser/parser.h"

namespace dbc_parser {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;

// Forward declarations
class ParserContext;

// Temporary structures for parsing
struct VersionStruct {
  std::string version_string;
};

struct NewSymbolsStruct {
  std::vector<std::string> symbols;
};

struct BitTimingStruct {
  uint32_t baudrate;
  uint32_t BTR1;
  uint32_t BTR2;
};

struct NodeStruct {
  std::string name;
  std::string comment;
};

struct SignalStruct {
  std::string name;
  uint32_t start_bit;
  uint32_t length;
  bool is_little_endian;
  bool is_signed;
  double factor;
  double offset;
  double min_value;
  double max_value;
  std::string unit;
  std::vector<std::string> receivers;
  MultiplexerType mux_type;
  uint32_t mux_value;
  std::map<int64_t, std::string> value_descriptions;
  std::string comment;
  SignalExtendedValueType extended_value_type;
};

struct MessageStruct {
  MessageId id;
  std::string name;
  uint32_t length;
  std::string sender;
  std::vector<SignalStruct> signals;
  std::string comment;
  std::vector<std::string> transmitters;
};

struct ValueDescriptionStruct {
  MessageId message_id;
  std::string signal_name;
  int64_t value;
  std::string description;
};

struct CommentStruct {
  std::string type;  // "BU_", "BO_", "SG_" etc.
  MessageId message_id;
  std::string signal_name;
  std::string node_name;
  std::string comment;
};

struct AttributeDefinitionStruct {
  std::string type;  // "BU_", "BO_", "SG_" etc.
  std::string name;
  AttributeType attr_type;
  boost::optional<double> minimum;
  boost::optional<double> maximum;
  boost::optional<std::string> default_value;
  std::vector<std::string> enum_values;
};

struct AttributeValueStruct {
  std::string type;  // "BU_", "BO_", "SG_" etc.
  MessageId message_id;
  std::string signal_name;
  std::string node_name;
  std::string attr_name;
  boost::variant<int64_t, double, std::string> value;
};

struct SignalGroupStruct {
  MessageId message_id;
  std::string name;
  uint32_t id;
  std::vector<std::string> signals;
};

// Implementation of the DBC grammar
template <typename Iterator, typename Skipper = ascii::space_type>
struct DbcGrammar : qi::grammar<Iterator, Skipper> {
  DbcGrammar(ParserContext& context, ParserErrorHandler& error_handler);
  
  // Main rule for the entire DBC file
  qi::rule<Iterator, Skipper> start;
  
  // Individual section rules
  qi::rule<Iterator, VersionStruct(), Skipper> version;
  qi::rule<Iterator, NewSymbolsStruct(), Skipper> new_symbols;
  qi::rule<Iterator, BitTimingStruct(), Skipper> bit_timing;
  qi::rule<Iterator, std::vector<NodeStruct>(), Skipper> nodes;
  qi::rule<Iterator, NodeStruct(), Skipper> node;
  qi::rule<Iterator, std::vector<MessageStruct>(), Skipper> messages;
  qi::rule<Iterator, MessageStruct(), Skipper> message;
  qi::rule<Iterator, SignalStruct(), Skipper> signal;
  qi::rule<Iterator, std::vector<CommentStruct>(), Skipper> comments;
  qi::rule<Iterator, CommentStruct(), Skipper> comment;
  qi::rule<Iterator, std::vector<ValueDescriptionStruct>(), Skipper> value_descriptions;
  qi::rule<Iterator, ValueDescriptionStruct(), Skipper> value_description;
  qi::rule<Iterator, std::vector<AttributeDefinitionStruct>(), Skipper> attribute_definitions;
  qi::rule<Iterator, AttributeDefinitionStruct(), Skipper> attribute_definition;
  qi::rule<Iterator, std::vector<AttributeValueStruct>(), Skipper> attribute_values;
  qi::rule<Iterator, AttributeValueStruct(), Skipper> attribute_value;
  qi::rule<Iterator, std::vector<SignalGroupStruct>(), Skipper> signal_groups;
  qi::rule<Iterator, SignalGroupStruct(), Skipper> signal_group;
  
  // Helper rules
  qi::rule<Iterator, uint32_t(), Skipper> message_id;
  qi::rule<Iterator, std::string(), Skipper> identifier;
  qi::rule<Iterator, std::string(), Skipper> quoted_string;
  qi::rule<Iterator, MultiplexerType(), Skipper> multiplexer_indicator;
  
  // Context to build the database
  ParserContext& context_;
  ParserErrorHandler& error_handler_;
};

// Context for building the database during parsing
class ParserContext {
 public:
  ParserContext();
  ~ParserContext();
  
  void set_version(const VersionStruct& version);
  void set_new_symbols(const NewSymbolsStruct& symbols);
  void set_bit_timing(const BitTimingStruct& bit_timing);
  void add_node(const NodeStruct& node);
  void add_message(const MessageStruct& message);
  void add_value_description(const ValueDescriptionStruct& value_desc);
  void add_comment(const CommentStruct& comment);
  void add_attribute_definition(const AttributeDefinitionStruct& attr_def);
  void add_attribute_value(const AttributeValueStruct& attr_value);
  void add_signal_group(const SignalGroupStruct& signal_group);
  
  std::unique_ptr<Database> finalize();
  
 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace dbc_parser

// Adapt structs for Boost Fusion
BOOST_FUSION_ADAPT_STRUCT(
  dbc_parser::VersionStruct,
  (std::string, version_string)
)

BOOST_FUSION_ADAPT_STRUCT(
  dbc_parser::NewSymbolsStruct,
  (std::vector<std::string>, symbols)
)

BOOST_FUSION_ADAPT_STRUCT(
  dbc_parser::BitTimingStruct,
  (uint32_t, baudrate)
  (uint32_t, BTR1)
  (uint32_t, BTR2)
)

BOOST_FUSION_ADAPT_STRUCT(
  dbc_parser::NodeStruct,
  (std::string, name)
  (std::string, comment)
)

BOOST_FUSION_ADAPT_STRUCT(
  dbc_parser::SignalStruct,
  (std::string, name)
  (uint32_t, start_bit)
  (uint32_t, length)
  (bool, is_little_endian)
  (bool, is_signed)
  (double, factor)
  (double, offset)
  (double, min_value)
  (double, max_value)
  (std::string, unit)
  (std::vector<std::string>, receivers)
  (dbc_parser::MultiplexerType, mux_type)
  (uint32_t, mux_value)
  (std::map<int64_t, std::string>, value_descriptions)
  (std::string, comment)
  (dbc_parser::SignalExtendedValueType, extended_value_type)
)

BOOST_FUSION_ADAPT_STRUCT(
  dbc_parser::MessageStruct,
  (dbc_parser::MessageId, id)
  (std::string, name)
  (uint32_t, length)
  (std::string, sender)
  (std::vector<dbc_parser::SignalStruct>, signals)
  (std::string, comment)
  (std::vector<std::string>, transmitters)
)

BOOST_FUSION_ADAPT_STRUCT(
  dbc_parser::CommentStruct,
  (std::string, type)
  (dbc_parser::MessageId, message_id)
  (std::string, signal_name)
  (std::string, node_name)
  (std::string, comment)
)

BOOST_FUSION_ADAPT_STRUCT(
  dbc_parser::ValueDescriptionStruct,
  (dbc_parser::MessageId, message_id)
  (std::string, signal_name)
  (int64_t, value)
  (std::string, description)
)

BOOST_FUSION_ADAPT_STRUCT(
  dbc_parser::AttributeDefinitionStruct,
  (std::string, type)
  (std::string, name)
  (dbc_parser::AttributeType, attr_type)
  (boost::optional<double>, minimum)
  (boost::optional<double>, maximum)
  (boost::optional<std::string>, default_value)
  (std::vector<std::string>, enum_values)
)

BOOST_FUSION_ADAPT_STRUCT(
  dbc_parser::AttributeValueStruct,
  (std::string, type)
  (dbc_parser::MessageId, message_id)
  (std::string, signal_name)
  (std::string, node_name)
  (std::string, attr_name)
  (boost::variant<int64_t, double, std::string>, value)
)

BOOST_FUSION_ADAPT_STRUCT(
  dbc_parser::SignalGroupStruct,
  (dbc_parser::MessageId, message_id)
  (std::string, name)
  (uint32_t, id)
  (std::vector<std::string>, signals)
)

#endif // DBC_PARSER_DBC_GRAMMAR_H_ 