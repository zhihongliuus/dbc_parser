#ifndef DBC_PARSER_DBC_GRAMMAR_H_
#define DBC_PARSER_DBC_GRAMMAR_H_

#define BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_PHOENIX_NO_PREDEFINED_TERMINALS
#define BOOST_PHOENIX_LIMIT 10
#define PHOENIX_LIMIT 10

#include "dbc_parser/types.h"

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>
#include <functional>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/phoenix/object.hpp>
#include <boost/phoenix/bind.hpp>
#include <boost/phoenix/stl.hpp>
#include <boost/phoenix/fusion.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace dbc_parser {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

// Forward declarations
class Database;

// Type definitions
using MessageId = uint32_t;

// Temporary structures for parsing
struct VersionStruct {
    std::string version_string;
    
    // Constructor that takes a string
    explicit VersionStruct(const std::string& version) : version_string(version) {}
    VersionStruct() = default;
};

struct NewSymbolsStruct {
    std::vector<std::string> symbols;
};

struct BitTimingStruct {
    BitTimingStruct() : baudrate(0), BTR1(0), BTR2(0) {}
    BitTimingStruct(uint32_t b, uint32_t b1, uint32_t b2) 
        : baudrate(b), BTR1(b1), BTR2(b2) {}
    
    uint32_t baudrate;
    uint32_t BTR1;
    uint32_t BTR2;
};

struct NodeStruct {
    std::string name;
    std::string comment;
};

struct ValueTableStruct {
    std::string name;
    std::map<int64_t, std::string> values;
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

struct MessageTransmitterStruct {
    MessageId message_id;
    std::vector<std::string> transmitters;
};

struct EnvironmentVariableStruct {
    std::string name;
    uint32_t type;
    double min;
    double max;
    std::string unit;
    double initial_value;
    std::map<std::string, std::string> data_values;
};

struct SignalTypeStruct {
    std::string name;
    std::map<std::string, std::string> attributes;
};

struct CommentStruct {
    std::string type;
    MessageId message_id;
    std::string signal_name;
    std::string node_name;
    std::string comment;
};

struct ValueDescriptionStruct {
    MessageId message_id;
    std::string signal_name;
    int64_t value;
    std::string description;
};

struct AttributeDefinitionStruct {
    std::string type;
    std::string name;
    AttributeType attr_type;
    boost::optional<double> minimum;
    boost::optional<double> maximum;
    boost::optional<std::string> default_value;
    std::vector<std::string> enum_values;
};

struct AttributeDefaultStruct {
    std::string name;
    std::string value;
};

struct AttributeValueStruct {
    std::string type;
    MessageId message_id;
    std::string signal_name;
    std::string node_name;
    std::string attr_name;
    boost::variant<int64_t, double, std::string> value;
};

struct SignalExtendedValueTypeStruct {
    MessageId message_id;
    std::string signal_name;
    SignalExtendedValueType type;
};

struct SignalGroupStruct {
    MessageId message_id;
    std::string name;
    uint32_t id;
    std::vector<std::string> signals;
};

struct SignalTypeRefStruct {
    std::string signal_name;
    std::string type_name;
};

// Error handler interface
class ParserErrorHandler {
public:
    virtual ~ParserErrorHandler() = default;
    virtual void on_error(const std::string& message, int line, int column) = 0;
    virtual void on_warning(const std::string& message, int line, int column) = 0;
    virtual void on_info(const std::string& message, int line, int column) = 0;
};

// Default error handler implementation
class DefaultParserErrorHandler : public ParserErrorHandler {
public:
    void on_error(const std::string& message, int line, int column) override;
    void on_warning(const std::string& message, int line, int column) override;
    void on_info(const std::string& message, int line, int column) override;
};

// Parser context for building the database
class ParserContext {
public:
    ParserContext();
    ~ParserContext();
    
    void set_version(const VersionStruct& version);
    void add_new_symbol(const std::string& symbol);
    void set_bit_timing(const BitTimingStruct& bit_timing);
    void add_node(const NodeStruct& node);
    void add_value_table(const ValueTableStruct& value_table);
    void add_message(const MessageStruct& message);
    void add_message_transmitter(const MessageTransmitterStruct& transmitter);
    void add_environment_variable(const EnvironmentVariableStruct& env_var);
    void add_environment_variable_data(const EnvironmentVariableStruct& env_var_data);
    void add_signal_type(const SignalTypeStruct& signal_type);
    void add_comment(const CommentStruct& comment);
    void add_attribute_definition(const AttributeDefinitionStruct& attr_def);
    void add_attribute_default(const AttributeDefaultStruct& attr_default);
    void add_attribute_value(const AttributeValueStruct& attr_value);
    void add_value_description(const ValueDescriptionStruct& value_desc);
    void add_signal_extended_value_type(const SignalExtendedValueTypeStruct& ext_value_type);
    void add_signal_group(const SignalGroupStruct& signal_group);
    void add_signal_type_ref(const SignalTypeRefStruct& type_ref);
    
    std::unique_ptr<Database> finalize();
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// Boost.Spirit grammar for DBC parsing
template <typename Iterator, typename Skipper = qi::space_type>
class DbcGrammar : public qi::grammar<Iterator, Skipper> {
public:
    DbcGrammar(ParserContext& context, ParserErrorHandler& error_handler);

private:
    // Rule declarations
    qi::rule<Iterator, std::string(), qi::space_type> identifier;
    qi::rule<Iterator, std::string(), qi::space_type> quoted_string;
    qi::rule<Iterator, qi::space_type> version_rule;
    qi::rule<Iterator, qi::space_type> new_symbols_rule;
    qi::rule<Iterator, boost::spirit::qi::locals<uint32_t, uint32_t, uint32_t>, qi::space_type> bit_timing_rule;
    qi::rule<Iterator, qi::space_type> nodes_rule;
    qi::rule<Iterator, qi::space_type> value_tables_rule;
    qi::rule<Iterator, qi::space_type> messages_rule;
    qi::rule<Iterator, qi::space_type> message_transmitters_rule;
    qi::rule<Iterator, qi::space_type> environment_variables_rule;
    qi::rule<Iterator, qi::space_type> environment_variables_data_rule;
    qi::rule<Iterator, qi::space_type> signal_types_rule;
    qi::rule<Iterator, qi::space_type> comments_rule;
    qi::rule<Iterator, qi::space_type> attribute_definitions_rule;
    qi::rule<Iterator, qi::space_type> attribute_defaults_rule;
    qi::rule<Iterator, qi::space_type> attribute_values_rule;
    qi::rule<Iterator, qi::space_type> value_descriptions_rule;
    qi::rule<Iterator, qi::space_type> signal_extended_value_type_list_rule;
    qi::rule<Iterator, qi::space_type> signal_groups_rule;
    qi::rule<Iterator, qi::space_type> signal_multiplexer_value_rule;
    qi::rule<Iterator, qi::space_type> sigtype_attr_list_rule;
    qi::rule<Iterator, qi::space_type> signal_type_refs_rule;
    qi::rule<Iterator, qi::space_type> start;
    
    // Callback functions
    std::function<void(const std::string&)> on_version;
    std::function<void(const std::vector<std::string>&)> on_new_symbols;
    std::function<void(const BitTimingStruct&)> on_bit_timing;
    std::function<void(const std::vector<std::string>&)> on_nodes;
    
    ParserContext& context_;
    ParserErrorHandler& error_handler_;
};

} // namespace dbc_parser

#endif // DBC_PARSER_DBC_GRAMMAR_H_ 