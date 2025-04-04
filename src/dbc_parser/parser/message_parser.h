#ifndef SRC_DBC_PARSER_PARSER_MESSAGE_PARSER_H_
#define SRC_DBC_PARSER_PARSER_MESSAGE_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dbc_parser {
namespace parser {

// Signal byte order
enum class ByteOrder {
  kMotorola = 0,  // Big endian
  kIntel = 1      // Little endian
};

// Signal value sign type
enum class SignType {
  kUnsigned = 0,
  kSigned = 1
};

// Signal multiplexing type
enum class MultiplexType {
  kNone = 0,       // Not multiplexed
  kMultiplexor,    // This signal is the multiplexor
  kMultiplexed     // This signal is multiplexed
};

// Represents a signal within a message
struct Signal {
  std::string name;
  int start_bit = 0;
  int length = 0;
  int byte_order = 0;  // 0=Motorola (big-endian), 1=Intel (little-endian)
  SignType sign = SignType::kUnsigned;
  double factor = 1.0;
  double offset = 0.0;
  double minimum = 0.0;
  double maximum = 0.0;
  std::string unit;
  std::vector<std::string> receivers;
  
  // Multiplexing
  MultiplexType multiplex_type = MultiplexType::kNone;
  int multiplex_value = -1;  // Only relevant if multiplex_type is kMultiplexed
};

// Represents a message in the DBC file
struct Message {
  int id = 0;
  std::string name;
  int dlc = 0;  // Data Length Code (number of bytes)
  std::string sender;
  std::vector<Signal> signals;
};

// Parser for the Message (BO_) section of a DBC file
class MessageParser {
 public:
  // Parses a message section from the given input
  // Returns std::nullopt if parsing fails
  static std::optional<Message> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // SRC_DBC_PARSER_PARSER_MESSAGE_PARSER_H_ 