#define BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_PHOENIX_NO_PREDEFINED_TERMINALS
#include "../../include/dbc_parser/dbc_grammar.h"
#include "../../include/dbc_parser/types.h"

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

// Bit timing handler
struct bit_timing_handler {
    ParserContext& context;
    
    bit_timing_handler(ParserContext& ctx) : context(ctx) {}
    
    void operator()(BitTimingStruct bit_timing) const {
        context.set_bit_timing(bit_timing);
    }
};

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
  
  // Create and return an empty database for now
  // The complete implementation of this method will be filled in later
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
    
    // Basic rules
    identifier %= qi::lexeme[qi::alpha >> *(qi::alnum | qi::char_('_'))];
    quoted_string %= qi::lexeme['"' >> *(qi::char_ - '"') >> '"'];
    
    // Version rule
    version_rule = qi::lit("VERSION") > quoted_string
      [phoenix::bind(&ParserContext::set_version, phoenix::ref(context), 
        phoenix::construct<VersionStruct>(qi::_1))];
    
    // New symbols rule
    new_symbols_rule = qi::lit("NS_:") > *(identifier
      [phoenix::bind(&ParserContext::add_new_symbol, phoenix::ref(context), qi::_1)]);
    
    // Bit timing rule - simplified version without semantic actions
    bit_timing_rule = qi::lit("BS_:") > qi::uint_ > qi::char_(',') > qi::uint_ > qi::char_(',') > qi::uint_;
    
    // Nodes rule
    nodes_rule = qi::lit("BU_:") > *(identifier
      [phoenix::bind([&](const std::string& name) {
          NodeStruct node;
          node.name = name;
          context.add_node(node);
        }, qi::_1)]);
    
    // More rule definitions follow...
    
    // Main grammar
    start = 
      version_rule > 
      -new_symbols_rule >
      -bit_timing_rule >
      nodes_rule;
      /* Comment out these rules until we fix them
      *(value_tables_rule | 
        messages_rule | 
        message_transmitters_rule | 
        environment_variables_rule | 
        environment_variables_data_rule | 
        signal_types_rule | 
        comments_rule | 
        attribute_definitions_rule | 
        attribute_defaults_rule | 
        attribute_values_rule | 
        value_descriptions_rule | 
        signal_extended_value_type_list_rule | 
        signal_groups_rule | 
        signal_multiplexer_value_rule | 
        sigtype_attr_list_rule | 
        signal_type_refs_rule);
      */
}

// Explicit template instantiation
template class DbcGrammar<std::string::const_iterator, qi::space_type>;

} // namespace dbc_parser