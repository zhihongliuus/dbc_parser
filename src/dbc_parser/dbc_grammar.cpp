#include "dbc_parser/dbc_grammar.h"

#include <fstream>
#include <iostream>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace dbc_parser {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

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
  std::vector<ValueDescriptionStruct> value_descriptions_;
  std::vector<AttributeDefinitionStruct> attribute_definitions_;
  std::vector<AttributeValueStruct> attribute_values_;
  std::vector<SignalGroupStruct> signal_groups_;
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

void ParserContext::add_comment(const CommentStruct& comment) {
  impl_->comments_.push_back(comment);
}

void ParserContext::add_value_description(const ValueDescriptionStruct& value_desc) {
  impl_->value_descriptions_.push_back(value_desc);
}

void ParserContext::add_attribute_definition(const AttributeDefinitionStruct& attr_def) {
  impl_->attribute_definitions_.push_back(attr_def);
}

void ParserContext::add_attribute_value(const AttributeValueStruct& attr_value) {
  impl_->attribute_values_.push_back(attr_value);
}

void ParserContext::add_signal_group(const SignalGroupStruct& signal_group) {
  impl_->signal_groups_.push_back(signal_group);
}

std::unique_ptr<Database> ParserContext::finalize() {
  auto db = std::make_unique<Database>();
  
  // Set version
  Database::Version version;
  version.version = impl_->version_.version_string;
  db->set_version(version);
  
  // Set bit timing
  Database::BitTiming bit_timing;
  bit_timing.baudrate = impl_->bit_timing_.baudrate;
  bit_timing.btr1 = impl_->bit_timing_.BTR1;
  bit_timing.btr2 = impl_->bit_timing_.BTR2;
  db->set_bit_timing(bit_timing);
  
  // Add nodes
  for (const auto& node_struct : impl_->nodes_) {
    auto node = std::make_unique<Node>(node_struct.name);
    node->set_comment(node_struct.comment);
    db->add_node(std::move(node));
  }
  
  // Add messages
  for (const auto& msg_struct : impl_->messages_) {
    auto msg = std::make_unique<Message>(msg_struct.id, msg_struct.name, msg_struct.length, msg_struct.sender);
    // Add signals and other message properties
    db->add_message(std::move(msg));
  }
  
  return db;
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

// Template instantiation for the grammar
template <typename Iterator, typename Skipper>
class DbcGrammar : public qi::grammar<Iterator, Skipper> {
public:
  DbcGrammar(ParserContext& context, ParserErrorHandler& error_handler)
    : DbcGrammar::base_type(start), context_(context), error_handler_(error_handler) {
    
    // Define grammar rules
    identifier %= qi::lexeme[qi::alpha >> *(qi::alnum | qi::char_('_'))];
    quoted_string %= qi::lexeme['"' >> *(qi::char_ - '"') >> '"'];
    
    version %= qi::lit("VERSION") >> quoted_string;
    
    bit_timing = qi::lit("BS_:") >> 
                 qi::uint_[phoenix::bind(&BitTimingStruct::baudrate, qi::_val) = qi::_1] >> 
                 qi::uint_[phoenix::bind(&BitTimingStruct::BTR1, qi::_val) = qi::_1] >> 
                 qi::uint_[phoenix::bind(&BitTimingStruct::BTR2, qi::_val) = qi::_1];
    
    nodes %= qi::lit("BU_:") >> *identifier;
    
    // Handler for version
    on_version = [&](const std::string& version_string) {
      VersionStruct version;
      version.version_string = version_string;
      context.set_version(version);
    };
    
    // Handler for bit timing
    on_bit_timing = [&](const BitTimingStruct& bit_timing) {
      context.set_bit_timing(bit_timing);
    };
    
    // Handler for nodes
    on_nodes = [&](const std::vector<std::string>& node_names) {
      for (const auto& name : node_names) {
        NodeStruct node;
        node.name = name;
        context.add_node(node);
      }
    };
    
    // Define the start rule
    start = 
      version[on_version] >> 
      -bit_timing[on_bit_timing] >> 
      nodes[on_nodes];
    
    // Error handling
    qi::on_error<qi::fail>(
      start,
      [&](auto&, auto&, auto&, auto& what) {
        error_handler.on_error(what, 0, 0);
      }
    );
  }

private:
  ParserContext& context_;
  ParserErrorHandler& error_handler_;
  
  // Phoenix handlers
  phoenix::function<std::function<void(const std::string&)>> on_version;
  phoenix::function<std::function<void(const BitTimingStruct&)>> on_bit_timing;
  phoenix::function<std::function<void(const std::vector<std::string>&)>> on_nodes;
  
  // Rules
  qi::rule<Iterator, std::string(), Skipper> identifier;
  qi::rule<Iterator, std::string(), Skipper> quoted_string;
  qi::rule<Iterator, std::string(), Skipper> version;
  qi::rule<Iterator, BitTimingStruct(), Skipper> bit_timing;
  qi::rule<Iterator, std::vector<std::string>(), Skipper> nodes;
  qi::rule<Iterator, Skipper> start;
};

// Explicit template instantiation
template class DbcGrammar<std::string::const_iterator, qi::space_type>;

} // namespace dbc_parser 