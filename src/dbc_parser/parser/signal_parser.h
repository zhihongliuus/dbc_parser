#ifndef DBC_PARSER_PARSER_SIGNAL_PARSER_H_
#define DBC_PARSER_PARSER_SIGNAL_PARSER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dbc_parser {
namespace parser {

// Structure to hold signal data from DBC file
struct Signal {
  std::string name;                   // Signal name
  int start_bit = 0;                  // Start bit position
  int signal_size = 0;                // Size in bits
  bool is_little_endian = true;       // Byte order (1=little endian, 0=big endian)
  bool is_signed = false;             // Sign (+ or -)
  double factor = 1.0;                // Scaling factor
  double offset = 0.0;                // Offset
  double minimum = 0.0;               // Minimum value
  double maximum = 0.0;               // Maximum value
  std::string unit;                   // Unit (e.g., "km/h")
  std::vector<std::string> receivers; // Receiving nodes
  bool is_multiplexer = false;        // Whether signal is a multiplexer switch
  std::optional<int> multiplexed_by;  // Multiplexer value if signal is multiplexed
};

// Parser for SG_ entries in DBC files
class SignalParser {
 public:
  // Parses a signal string and returns a Signal object if parsing is successful
  // Returns std::nullopt if parsing fails
  //
  // Example format:
  // SG_ SignalName : 8|16@1+ (0.1,0) [0|655.35] "km/h" ECU1,ECU2
  static std::optional<Signal> Parse(std::string_view input);
};

}  // namespace parser
}  // namespace dbc_parser

#endif  // DBC_PARSER_PARSER_SIGNAL_PARSER_H_ 