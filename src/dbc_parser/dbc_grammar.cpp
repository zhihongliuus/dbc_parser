#define BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_PHOENIX_NO_PREDEFINED_TERMINALS
#include "dbc_parser/dbc_grammar.h"

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/qi.hpp>
#include <fstream>
#include <iostream>

#include "dbc_parser/types.h"

// Free function for error handling
template <typename Iterator>
boost::spirit::qi::error_handler_result handle_error(
    Iterator first, Iterator last, Iterator where,
    const boost::spirit::info& what) {
  std::cerr << "Error: expecting " << what.tag << " at "
            << std::string(where, last) << std::endl;
  return boost::spirit::qi::fail;
}

namespace dbc_parser {

// Implementation of DefaultParserErrorHandler
void DefaultParserErrorHandler::on_error(const std::string& message, int line,
                                         int column) {
  std::cerr << "Error at line " << line << ", column " << column << ": "
            << message << std::endl;
}

void DefaultParserErrorHandler::on_warning(const std::string& message, int line,
                                           int column) {
  std::cerr << "Warning at line " << line << ", column " << column << ": "
            << message << std::endl;
}

void DefaultParserErrorHandler::on_info(const std::string& message, int line,
                                        int column) {
  std::cerr << "Info at line " << line << ", column " << column << ": "
            << message << std::endl;
}

// ParserContext implementation
class ParserContext::Impl {
 public:
  Impl() = default;

  std::string version;
  std::vector<std::string> new_symbols;
  BitTimingStruct bit_timing;
  std::vector<NodeStruct> nodes;
  std::vector<ValueTableStruct> value_tables;
  std::vector<MessageStruct> messages;
  std::vector<MessageTransmitterStruct> message_transmitters;
  std::vector<EnvironmentVariableStruct> environment_variables;
  std::vector<EnvironmentVariableStruct> environment_variable_data;
  std::vector<SignalTypeStruct> signal_types;
  std::vector<CommentStruct> comments;
  std::vector<AttributeDefinitionStruct> attribute_definitions;
  std::vector<AttributeDefaultStruct> attribute_defaults;
  std::vector<AttributeValueStruct> attribute_values;
  std::vector<ValueDescriptionStruct> value_descriptions;
  std::vector<SignalExtendedValueTypeStruct> signal_extended_value_types;
  std::vector<SignalGroupStruct> signal_groups;
  std::vector<SignalTypeRefStruct> signal_type_refs;
};

ParserContext::ParserContext() : impl_(std::make_unique<Impl>()) {}
ParserContext::~ParserContext() = default;

void ParserContext::set_version(const VersionStruct& version) {
  impl_->version = version.version_string;
}

void ParserContext::add_new_symbol(const std::string& symbol) {
  impl_->new_symbols.push_back(symbol);
}

void ParserContext::set_bit_timing(const BitTimingStruct& bit_timing) {
  impl_->bit_timing = bit_timing;
}

void ParserContext::add_node(const NodeStruct& node) {
  impl_->nodes.push_back(node);
}

void ParserContext::add_value_table(const ValueTableStruct& value_table) {
  impl_->value_tables.push_back(value_table);
}

void ParserContext::add_message(const MessageStruct& message) {
  impl_->messages.push_back(message);
}

void ParserContext::add_message_transmitter(
    const MessageTransmitterStruct& transmitter) {
  impl_->message_transmitters.push_back(transmitter);
}

void ParserContext::add_environment_variable(
    const EnvironmentVariableStruct& env_var) {
  impl_->environment_variables.push_back(env_var);
}

void ParserContext::add_environment_variable_data(
    const EnvironmentVariableStruct& env_var_data) {
  impl_->environment_variable_data.push_back(env_var_data);
}

void ParserContext::add_signal_type(const SignalTypeStruct& signal_type) {
  impl_->signal_types.push_back(signal_type);
}

void ParserContext::add_comment(const CommentStruct& comment) {
  impl_->comments.push_back(comment);
}

void ParserContext::add_attribute_definition(
    const AttributeDefinitionStruct& attr_def) {
  impl_->attribute_definitions.push_back(attr_def);
}

void ParserContext::add_attribute_default(
    const AttributeDefaultStruct& attr_default) {
  impl_->attribute_defaults.push_back(attr_default);
}

void ParserContext::add_attribute_value(
    const AttributeValueStruct& attr_value) {
  impl_->attribute_values.push_back(attr_value);
}

void ParserContext::add_value_description(
    const ValueDescriptionStruct& value_desc) {
  impl_->value_descriptions.push_back(value_desc);
}

void ParserContext::add_signal_extended_value_type(
    const SignalExtendedValueTypeStruct& ext_value_type) {
  impl_->signal_extended_value_types.push_back(ext_value_type);
}

void ParserContext::add_signal_group(const SignalGroupStruct& signal_group) {
  impl_->signal_groups.push_back(signal_group);
}

void ParserContext::add_signal_type_ref(const SignalTypeRefStruct& type_ref) {
  impl_->signal_type_refs.push_back(type_ref);
}

std::unique_ptr<Database> ParserContext::finalize() {
  auto db = std::make_unique<Database>();

  // Set version
  if (!impl_->version.empty()) {
    Database::Version version;
    version.version = impl_->version;
    db->set_version(version);
  }

  // Set new symbols
  db->set_new_symbols(impl_->new_symbols);

  // Set bit timing
  if (impl_->bit_timing.baudrate > 0) {
    Database::BitTiming bit_timing;
    bit_timing.baudrate = impl_->bit_timing.baudrate;
    bit_timing.btr1 = impl_->bit_timing.BTR1;
    bit_timing.btr2 = impl_->bit_timing.BTR2;
    db->set_bit_timing(bit_timing);
  }

  // Add nodes
  for (const auto& node : impl_->nodes) {
    auto db_node = std::make_unique<Node>(node.name);
    db_node->set_comment(node.comment);
    db->add_node(std::move(db_node));
  }

  // Add value tables
  for (const auto& value_table : impl_->value_tables) {
    auto db_value_table = std::make_unique<ValueTable>(value_table.name);
    for (const auto& [value, description] : value_table.values) {
      db_value_table->add_value(value, description);
    }
    db->add_value_table(std::move(db_value_table));
  }

  // Add messages
  for (const auto& message : impl_->messages) {
    auto db_message = std::make_unique<Message>(message.id, message.name,
                                                message.length, message.sender);
    db_message->set_comment(message.comment);

    // Add transmitters to message
    for (const auto& transmitter : message.transmitters) {
      db_message->add_transmitter(transmitter);
    }

    db->add_message(std::move(db_message));
  }

  // Add message transmitters
  for (const auto& transmitter : impl_->message_transmitters) {
    // Find the message by ID
    auto* message = db->get_message(transmitter.message_id);
    if (message) {
      // Add transmitters to the message
      for (const auto& transmitter_name : transmitter.transmitters) {
        message->add_transmitter(transmitter_name);
      }
    }
  }

  // Add environment variables
  for (const auto& env_var : impl_->environment_variables) {
    // In a real implementation, we would create EnvironmentVariable objects
    // here and add them to the database
  }

  // Add comments
  for (const auto& comment : impl_->comments) {
    if (comment.type == "BU_") {
      // Node comment
      auto* node = db->get_node(comment.node_name);
      if (node) {
        node->set_comment(comment.comment);
      }
    } else if (comment.type == "BO_") {
      // Message comment
      auto* message = db->get_message(comment.message_id);
      if (message) {
        message->set_comment(comment.comment);
      }
    } else if (comment.type == "SG_") {
      // Signal comment
      auto* message = db->get_message(comment.message_id);
      if (message) {
        auto* signal = message->get_signal(comment.signal_name);
        if (signal) {
          signal->set_comment(comment.comment);
        }
      }
    }
  }

  // Add value descriptions
  for (const auto& value_desc : impl_->value_descriptions) {
    auto* message = db->get_message(value_desc.message_id);
    if (message) {
      auto* signal = message->get_signal(value_desc.signal_name);
      if (signal) {
        signal->add_value_description(value_desc.value, value_desc.description);
      }
    }
  }

  // Add signal groups
  for (const auto& signal_group : impl_->signal_groups) {
    auto* message = db->get_message(signal_group.message_id);
    if (message) {
      auto db_signal_group = std::make_unique<SignalGroup>(
          signal_group.message_id, signal_group.name, signal_group.id);

      // Add signals to the group
      for (const auto& signal_name : signal_group.signals) {
        db_signal_group->add_signal(signal_name);
      }

      message->add_signal_group(std::move(db_signal_group));
    }
  }

  return db;
}

}  // namespace dbc_parser

// Adapt structs for Boost.Fusion
BOOST_FUSION_ADAPT_STRUCT(dbc_parser::VersionStruct,
                          (std::string, version_string))

BOOST_FUSION_ADAPT_STRUCT(dbc_parser::BitTimingStruct,
                          (uint32_t, baudrate)(uint32_t, BTR1)(uint32_t, BTR2))

BOOST_FUSION_ADAPT_STRUCT(dbc_parser::NodeStruct,
                          (std::string, name)(std::string, comment))

BOOST_FUSION_ADAPT_STRUCT(dbc_parser::ValueTableStruct, (std::string, name))

// Adapt only the necessary fields for SignalStruct
BOOST_FUSION_ADAPT_STRUCT(
    dbc_parser::SignalStruct,
    (std::string, name)(uint32_t, start_bit)(uint32_t,
                                             length)(bool, is_little_endian)(
        bool, is_signed)(double, factor)(double, offset)(double, min_value)(
        double, max_value)(std::string, unit)(dbc_parser::MultiplexerType,
                                              mux_type)(uint32_t, mux_value))

// Adapt only the necessary fields for MessageStruct
BOOST_FUSION_ADAPT_STRUCT(dbc_parser::MessageStruct,
                          (dbc_parser::MessageId,
                           id)(std::string, name)(uint32_t, length)(std::string,
                                                                    sender))

namespace dbc_parser {

// DbcGrammar implementation
template <typename Iterator, typename Skipper>
DbcGrammar<Iterator, Skipper>::DbcGrammar(ParserContext& context,
                                          ParserErrorHandler& error_handler)
    : DbcGrammar::base_type(dbc_file, "dbc_file"),
      context_(context),
      error_handler_(error_handler) {
  namespace qi = boost::spirit::qi;
  namespace phoenix = boost::phoenix;

  using phoenix::bind;
  using phoenix::construct;
  using phoenix::ref;
  using phoenix::val;
  using qi::_1;
  using qi::_2;
  using qi::_3;
  using qi::_4;
  using qi::_5;
  using qi::_6;
  using qi::_7;
  using qi::_8;
  using qi::_9;
  using qi::_a;
  using qi::_b;
  using qi::_c;
  using qi::_d;
  using qi::_r1;
  using qi::_val;
  using qi::char_;
  using qi::double_;
  using qi::eol;
  using qi::eps;
  using qi::fail;
  using qi::int_;
  using qi::lexeme;
  using qi::lit;
  using qi::on_error;
  using qi::uint_;

  // Basic parsers
  identifier = lexeme[char_("a-zA-Z_") >> *char_("a-zA-Z0-9_")];
  identifier.name("identifier");

  quoted_string = lexeme['"' >> *(char_ - '"') >> '"'];
  quoted_string.name("quoted_string");

  // Version rule
  version_rule =
      lit("VERSION") >> quoted_string[phoenix::bind(
                            &ParserContext::set_version, phoenix::ref(context_),
                            phoenix::construct<VersionStruct>(_1))];
  version_rule.name("version_rule");

  // New symbols rule
  new_symbols_rule = lit("NS_") >> lit(":") >>
                     *(identifier[phoenix::bind(&ParserContext::add_new_symbol,
                                                phoenix::ref(context_), _1)]);
  new_symbols_rule.name("new_symbols_rule");

  // Bit timing rule
  bit_timing_rule =
      lit("BS_") >> lit(":") >>
      (uint_ >> lit(",") >> uint_ >> lit(",") >> uint_)[phoenix::bind(
          &ParserContext::set_bit_timing, phoenix::ref(context_),
          phoenix::construct<BitTimingStruct>(_1, _2, _3))];
  bit_timing_rule.name("bit_timing_rule");

  // Node definition
  node_def = identifier[phoenix::at_c<0>(_val) = _1] >>
             -(lit(":") >> quoted_string[phoenix::at_c<1>(_val) = _1]);
  node_def.name("node_def");

  // Nodes rule
  nodes_rule = lit("BU_") >> lit(":") >>
               *(identifier[phoenix::bind(
                   &ParserContext::add_node, phoenix::ref(context_),
                   phoenix::construct<NodeStruct>(_1, ""))]);
  nodes_rule.name("nodes_rule");

  // Value table definition
  value_table_def =
      lit("VAL_TABLE_") >> identifier[phoenix::at_c<0>(_val) = _1] >>
      *(int_[_a = _1] >>
        quoted_string[phoenix::bind(
            [](ValueTableStruct& vt, int64_t value, const std::string& desc) {
              vt.values[value] = desc;
            },
            _val, _a, _1)]);
  value_table_def.name("value_table_def");

  // Value tables rule
  value_tables_rule = *(value_table_def[phoenix::bind(
      &ParserContext::add_value_table, phoenix::ref(context_), _1)]);
  value_tables_rule.name("value_tables_rule");

  // Signal definition with proper multiplexing support
  signal_def =
      lit("SG_") >> identifier[phoenix::at_c<0>(_val) = _1] >>
      // Handle multiplexing
      (
          // Multiplexor signal
          (lit("M") >> lit(":") >>
           eps[phoenix::at_c<10>(_val) = MultiplexerType::kMultiplexor]) |
          // Multiplexed signal
          (lit("m") >> uint_[phoenix::at_c<11>(_val) = _1] >> lit(":") >>
           eps[phoenix::at_c<10>(_val) = MultiplexerType::kMultiplexed]) |
          // Regular signal
          (lit(":") >>
           eps[phoenix::at_c<10>(_val) = MultiplexerType::kNone])) >>
      // Start bit, length, endianness, signedness
      uint_[phoenix::at_c<1>(_val) = _1] >> lit("|") >>
      uint_[phoenix::at_c<2>(_val) = _1] >> lit("@") >>
      uint_ >>  // Byte order (always 1 in DBC)
      (lit("+")[phoenix::at_c<3>(_val) = true] |
       lit("-")[phoenix::at_c<3>(_val) = false]) >>
      (lit("+")[phoenix::at_c<4>(_val) = false] |
       lit("-")[phoenix::at_c<4>(_val) = true]) >>
      // Factor, offset, min, max, unit
      lit("(") >> double_[phoenix::at_c<5>(_val) = _1] >> lit(",") >>
      double_[phoenix::at_c<6>(_val) = _1] >> lit(")") >> lit("[") >>
      double_[phoenix::at_c<7>(_val) = _1] >> lit("|") >>
      double_[phoenix::at_c<8>(_val) = _1] >> lit("]") >>
      quoted_string[phoenix::at_c<9>(_val) = _1] >>
      // Receivers
      *identifier[phoenix::bind(
          [](SignalStruct& signal, const std::string& receiver) {
            signal.receivers.push_back(receiver);
          },
          _val, _1)];
  signal_def.name("signal_def");

  // Message definition
  message_def = lit("BO_") >> uint_[phoenix::at_c<0>(_val) = _1] >>
                identifier[phoenix::at_c<1>(_val) = _1] >> lit(":") >>
                uint_[phoenix::at_c<2>(_val) = _1] >>
                identifier[phoenix::at_c<3>(_val) = _1];
  message_def.name("message_def");

  // Messages rule
  messages_rule = *(message_def[phoenix::bind(&ParserContext::add_message,
                                              phoenix::ref(context_), _1)]);
  messages_rule.name("messages_rule");

  // Message transmitters rule - simplified
  message_transmitters_rule =
      *(lit("BO_TX_BU_") >> uint_ >> lit(":") >> *identifier);
  message_transmitters_rule.name("message_transmitters_rule");

  // Environment variables rule - simplified
  environment_variables_rule =
      *(lit("EV_") >> identifier >> lit(":") >> uint_ >> lit("[") >> double_ >>
        lit("|") >> double_ >> lit("]") >> quoted_string >> double_ >>
        *identifier);
  environment_variables_rule.name("environment_variables_rule");

  // Environment variables data rule - simplified
  environment_variables_data_rule =
      *(lit("ENVVAR_DATA_") >> identifier >> lit(":") >> uint_);
  environment_variables_data_rule.name("environment_variables_data_rule");

  // Signal types rule - simplified
  signal_types_rule = *(lit("SGTYPE_") >> identifier >> lit(":") >> uint_ >>
                        *(identifier >> lit(":") >> identifier));
  signal_types_rule.name("signal_types_rule");

  // Comments rule - simplified with locals
  qi::rule<Iterator, qi::locals<std::string, uint32_t, std::string>,
           qi::space_type>
      comments_rule_with_locals =
          lit("CM_") >>
          ((lit("BU_") >> identifier[_a = "BU_", _c = _1] >> quoted_string) |
           (lit("BO_") >> uint_[_a = "BO_", _b = _1] >> quoted_string) |
           (lit("SG_") >> uint_[_a = "SG_", _b = _1] >> identifier[_c = _1] >>
            quoted_string) |
           (quoted_string[_a = "", _b = 0, _c = ""]));

  comments_rule = *comments_rule_with_locals;
  comments_rule.name("comments_rule");

  // Attribute definitions rule - simplified
  qi::rule<Iterator, qi::locals<std::string, std::string>, qi::space_type>
      attribute_def_rule_with_locals =
          lit("BA_DEF_") >> ((lit("BU_") >> identifier[_a = "BU_", _b = _1]) |
                             (lit("BO_") >> identifier[_a = "BO_", _b = _1]) |
                             (lit("SG_") >> identifier[_a = "SG_", _b = _1]) |
                             (eps[_a = "", _b = ""])) >>
          identifier >>
          ((lit("INT") >> int_ >> int_) | (lit("FLOAT") >> double_ >> double_) |
           (lit("STRING")) | (lit("ENUM") >> *(quoted_string)));

  attribute_definitions_rule = *attribute_def_rule_with_locals;
  attribute_definitions_rule.name("attribute_definitions_rule");

  // Attribute defaults rule - simplified
  attribute_defaults_rule =
      *(lit("BA_DEF_DEF_") >> identifier >> (int_ | double_ | quoted_string));
  attribute_defaults_rule.name("attribute_defaults_rule");

  // Attribute values rule - simplified
  qi::rule<Iterator, qi::locals<std::string, uint32_t, std::string>,
           qi::space_type>
      attribute_value_rule_with_locals =
          lit("BA_") >> identifier >>
          ((lit("BU_") >> identifier[_a = "BU_", _c = _1]) |
           (lit("BO_") >> uint_[_a = "BO_", _b = _1]) |
           (lit("SG_") >> uint_[_a = "SG_", _b = _1] >> identifier[_c = _1]) |
           (eps[_a = "", _b = 0, _c = ""])) >>
          (int_ | double_ | quoted_string);

  attribute_values_rule = *attribute_value_rule_with_locals;
  attribute_values_rule.name("attribute_values_rule");

  // Value descriptions rule - simplified
  value_descriptions_rule =
      *(lit("VAL_") >> uint_ >> identifier >> *(int_ >> quoted_string));
  value_descriptions_rule.name("value_descriptions_rule");

  // Signal extended value type rule - simplified
  signal_extended_value_types_rule =
      *(lit("SIG_VALTYPE_") >> uint_ >> identifier >> lit(":") >> uint_);
  signal_extended_value_types_rule.name("signal_extended_value_types_rule");

  // Signal groups rule - simplified
  signal_groups_rule = *(lit("SIG_GROUP_") >> uint_ >> identifier >> uint_ >>
                         lit(":") >> *identifier);
  signal_groups_rule.name("signal_groups_rule");

  // Signal type refs rule - simplified
  signal_type_refs_rule = *(lit("SIG_TYPE_REF_") >> identifier >> identifier);
  signal_type_refs_rule.name("signal_type_refs_rule");

  // DBC file - complete grammar with all rules
  dbc_file = version_rule >> new_symbols_rule >> bit_timing_rule >>
             nodes_rule >> value_tables_rule >> messages_rule >>
             message_transmitters_rule >> environment_variables_rule >>
             environment_variables_data_rule >> signal_types_rule >>
             comments_rule >> attribute_definitions_rule >>
             attribute_defaults_rule >> attribute_values_rule >>
             value_descriptions_rule >> signal_extended_value_types_rule >>
             signal_groups_rule >> signal_type_refs_rule;
  dbc_file.name("dbc_file");

  // Error handling with phoenix::bind to a free function
  on_error<boost::spirit::qi::fail>(
      dbc_file, phoenix::bind(&handle_error<Iterator>, _1, _2, _3, _4));
}

}  // namespace dbc_parser