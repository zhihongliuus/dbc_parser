#ifndef DBC_PARSER_DBC_GRAMMAR_H_
#define DBC_PARSER_DBC_GRAMMAR_H_

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include "dbc_parser/types.h"

namespace dbc_parser {

// Forward declarations
class Database;

// Type definitions
using MessageId = uint32_t;

// Temporary structures for parsing
struct VersionStruct {
  std::string version_string;
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

struct AttributeValueStruct {
  std::string type;
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

// Forward declaration of the grammar
template <typename Iterator, typename Skipper>
class DbcGrammar;

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
  void add_message(const MessageStruct& message);
  void add_comment(const CommentStruct& comment);
  void add_value_description(const ValueDescriptionStruct& value_desc);
  void add_attribute_definition(const AttributeDefinitionStruct& attr_def);
  void add_attribute_value(const AttributeValueStruct& attr_value);
  void add_signal_group(const SignalGroupStruct& signal_group);
  
  std::unique_ptr<Database> finalize();
  
private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace dbc_parser

#endif // DBC_PARSER_DBC_GRAMMAR_H_ 