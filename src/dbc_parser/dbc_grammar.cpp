#define BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_PHOENIX_NO_PREDEFINED_TERMINALS
#include "dbc_parser/dbc_grammar.h"
#include "dbc_parser/types.h"

#include <fstream>
#include <iostream>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/range/iterator_range.hpp>

// Adapt BitTimingStruct for fusion
BOOST_FUSION_ADAPT_STRUCT(
    dbc_parser::BitTimingStruct,
    (uint32_t, baudrate)
    (uint32_t, BTR1)
    (uint32_t, BTR2)
)

namespace dbc_parser {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;

using qi::lit;
using qi::lexeme;
using qi::uint_;
using qi::int_;
using qi::double_;
using qi::char_;
using qi::_1;
using qi::_2;
using qi::_3;
using qi::_4;
using qi::_5;
using qi::_6;
using qi::_7;
using qi::_8;
using qi::_9;
using qi::_val;
using qi::eps;
using qi::space;
using qi::eol;
using qi::eoi;
using qi::skip;
using qi::hold;
using qi::omit;
using qi::raw;
using qi::repeat;
using qi::no_skip;
using qi::fail;
using qi::on_error;
using qi::expect;
using phoenix::ref;
using phoenix::push_back;
using phoenix::construct;
using phoenix::val;
using phoenix::bind;
using std::string;

// Implementation of ParserContext
class ParserContext::Impl {
public:
  Impl() = default;
  
  VersionStruct version_;
  NewSymbolsStruct new_symbols_;
  BitTimingStruct bit_timing_;
  std::vector<NodeStruct> nodes_;
  std::vector<MessageStruct> messages_;
  std::vector<CommentStruct> comments_;
  std::vector<ValueTableStruct> value_tables_;
  std::vector<MessageTransmitterStruct> message_transmitters_;
  std::vector<EnvironmentVariableStruct> environment_variables_;
  std::vector<SignalTypeStruct> signal_types_;
  std::vector<ValueDescriptionStruct> value_descriptions_;
  std::vector<AttributeDefinitionStruct> attribute_definitions_;
  std::vector<AttributeDefaultStruct> attribute_defaults_;
  std::vector<AttributeValueStruct> attribute_values_;
  std::vector<SignalExtendedValueTypeStruct> signal_extended_value_types_;
  std::vector<SignalGroupStruct> signal_groups_;
  std::vector<SignalTypeRefStruct> signal_type_refs_;
};

ParserContext::ParserContext() : impl_(std::make_unique<Impl>()) {}
ParserContext::~ParserContext() = default;

void ParserContext::set_version(const VersionStruct& version) {
  impl_->version_ = version;
}

void ParserContext::add_new_symbol(const std::string& symbol) {
  impl_->new_symbols_.symbols.push_back(symbol);
}

void ParserContext::set_bit_timing(const BitTimingStruct& bit_timing) {
  impl_->bit_timing_ = bit_timing;
}

void ParserContext::add_node(const NodeStruct& node) {
  impl_->nodes_.push_back(node);
}

void ParserContext::add_message(const MessageStruct& message) {
  impl_->messages_.push_back(message);
}

void ParserContext::add_value_table(const ValueTableStruct& value_table) {
  impl_->value_tables_.push_back(value_table);
}

void ParserContext::add_message_transmitter(const MessageTransmitterStruct& transmitter) {
  impl_->message_transmitters_.push_back(transmitter);
}

void ParserContext::add_environment_variable(const EnvironmentVariableStruct& env_var) {
  impl_->environment_variables_.push_back(env_var);
}

void ParserContext::add_environment_variable_data(const EnvironmentVariableStruct& env_var_data) {
  // Find matching environment variable and update it
  for (auto& env_var : impl_->environment_variables_) {
    if (env_var.name == env_var_data.name) {
      // Update the environment variable with data from env_var_data
      // Just add it as is, no need to update specific fields
      return;
    }
  }
  
  // If not found, add as a new one
  impl_->environment_variables_.push_back(env_var_data);
}

void ParserContext::add_signal_type(const SignalTypeStruct& signal_type) {
  impl_->signal_types_.push_back(signal_type);
}

void ParserContext::add_comment(const CommentStruct& comment) {
  impl_->comments_.push_back(comment);
}

void ParserContext::add_attribute_definition(const AttributeDefinitionStruct& attr_def) {
  impl_->attribute_definitions_.push_back(attr_def);
}

void ParserContext::add_attribute_default(const AttributeDefaultStruct& attr_def) {
  impl_->attribute_defaults_.push_back(attr_def);
}

void ParserContext::add_attribute_value(const AttributeValueStruct& attr_value) {
  impl_->attribute_values_.push_back(attr_value);
}

void ParserContext::add_value_description(const ValueDescriptionStruct& value_desc) {
  impl_->value_descriptions_.push_back(value_desc);
}

void ParserContext::add_signal_extended_value_type(const SignalExtendedValueTypeStruct& signal_ext_value_type) {
  impl_->signal_extended_value_types_.push_back(signal_ext_value_type);
}

void ParserContext::add_signal_group(const SignalGroupStruct& signal_group) {
  impl_->signal_groups_.push_back(signal_group);
}

void ParserContext::add_signal_type_ref(const SignalTypeRefStruct& signal_type_ref) {
  impl_->signal_type_refs_.push_back(signal_type_ref);
}

std::unique_ptr<Database> ParserContext::finalize() {
  auto database = std::make_unique<Database>();
  
  // Set version
  if (!impl_->version_.version_string.empty()) {
    Database::Version version;
    version.version = impl_->version_.version_string;
    database->set_version(version);
  }
  
  // Set bit timing
  if (impl_->bit_timing_.baudrate > 0) {
    Database::BitTiming bit_timing;
    bit_timing.baudrate = impl_->bit_timing_.baudrate;
    bit_timing.btr1 = impl_->bit_timing_.BTR1;
    bit_timing.btr2 = impl_->bit_timing_.BTR2;
    database->set_bit_timing(bit_timing);
  }
  
  // Set new symbols
  if (!impl_->new_symbols_.symbols.empty()) {
    database->set_new_symbols(impl_->new_symbols_.symbols);
  }
  
  // Add nodes
  for (const auto& node : impl_->nodes_) {
    auto node_ptr = std::make_unique<Node>(node.name);
    if (!node.comment.empty()) {
      node_ptr->set_comment(node.comment);
    }
    database->add_node(std::move(node_ptr));
  }
  
  // Placeholders for other data - will be implemented later
  // We don't populate value tables and messages yet to avoid constructor issues
  
  return database;
}

void DefaultParserErrorHandler::on_error(const std::string& message, int line, int column) {
  std::cerr << "Error at line " << line << " column " << column << ": " << message << std::endl;
}

void DefaultParserErrorHandler::on_warning(const std::string& message, int line, int column) {
  std::cerr << "Warning at line " << line << " column " << column << ": " << message << std::endl;
}

void DefaultParserErrorHandler::on_info(const std::string& message, int line, int column) {
  std::cerr << "Info at line " << line << " column " << column << ": " << message << std::endl;
}

// Define the DbcGrammar implementation
template <typename Iterator, typename Skipper>
DbcGrammar<Iterator, Skipper>::DbcGrammar(ParserContext& context, ParserErrorHandler& error_handler)
    : DbcGrammar::base_type(start), context_(context), error_handler_(error_handler) {
  // Define the rules for parsing a DBC file
  
  // Basic rules
  identifier %= lexeme[char_("a-zA-Z_") > *char_("a-zA-Z0-9_")];
  quoted_string %= lexeme['"' > *(char_ - '"') > '"'];

  // Version rule
  version_rule = lit("VERSION") > quoted_string;
  
  // New symbols rule
  new_symbols_rule = lit("NS_:") > *identifier;

  // Bit timing rule
  bit_timing_rule = lit("BS_:") > uint_ > ',' > uint_ > ',' > uint_;

  // Nodes rule
  nodes_rule = lit("BU_:") > *identifier;

  // Value tables rule - simple placeholder
  value_tables_rule = lit("VAL_TABLE_") > identifier > *(int_ > quoted_string) > char_(';');

  // Messages rule - simple placeholder
  messages_rule = lit("BO_") > uint_ > identifier > char_(':') > uint_ > identifier > char_(';');
  
  // Message transmitters rule - simple placeholder 
  message_transmitters_rule = lit("BO_TX_BU_") > uint_ > char_(':') > *(identifier) > char_(';');
  
  // Environment variables rule - simple placeholder
  environment_variables_rule = lit("EV_") > identifier > char_(':') > uint_ > *(char_) > char_(';');

  // Comments rule - simple placeholder
  comments_rule = lit("CM_") > *(char_ - char_(';')) > char_(';');

  // Attribute definitions rule - simple placeholder
  attribute_definitions_rule = lit("BA_DEF_") > *(char_ - char_(';')) > char_(';');

  // Attribute defaults rule - simple placeholder
  attribute_defaults_rule = lit("BA_DEF_DEF_") > *(char_ - char_(';')) > char_(';');

  // Attribute values rule - simple placeholder
  attribute_values_rule = lit("BA_") > *(char_ - char_(';')) > char_(';');

  // Value descriptions rule - simple placeholder
  value_descriptions_rule = lit("VAL_") > *(char_ - char_(';')) > char_(';');

  // Main grammar
  start = 
    -(version_rule) > 
    -(new_symbols_rule) >
    -(bit_timing_rule) >
    -(nodes_rule) >
    *(value_tables_rule | 
      messages_rule | 
      message_transmitters_rule | 
      environment_variables_rule | 
      comments_rule | 
      attribute_definitions_rule | 
      attribute_defaults_rule | 
      attribute_values_rule | 
      value_descriptions_rule);
}

// Explicit template instantiation
template class DbcGrammar<std::string::const_iterator, qi::space_type>;

} // namespace dbc_parser