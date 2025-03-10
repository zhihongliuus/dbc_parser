#ifndef DBC_PARSER_DECODER_H_
#define DBC_PARSER_DECODER_H_

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "dbc_parser/types.h"

namespace dbc_parser {

// Decoded signal structure
struct DecodedSignal {
  std::string name;
  double value;
  std::string unit;
  std::string description;
};

// Decoded message structure
struct DecodedMessage {
  MessageId id;
  std::string name;
  std::map<std::string, DecodedSignal> signals;
};

// Decoder options
struct DecoderOptions {
  bool verbose = false;
  bool ignore_unknown_ids = true;
};

// Decoder class for converting CAN frames to physical values
class Decoder {
 public:
  explicit Decoder(std::shared_ptr<Database> db, const DecoderOptions& options = DecoderOptions());
  ~Decoder();
  
  // Decode a CAN frame with a specific ID
  std::optional<DecodedMessage> decode_frame(MessageId id, const std::vector<uint8_t>& data) const;
  
  // Decode a CAN frame with a specific ID and extract a specific signal
  std::optional<DecodedSignal> decode_signal(MessageId id, const std::string& signal_name, 
                                           const std::vector<uint8_t>& data) const;
  
  // Get textual description for a value if it exists
  std::optional<std::string> get_value_description(MessageId id, const std::string& signal_name, 
                                                 double value) const;
  
 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace dbc_parser

#endif // DBC_PARSER_DECODER_H_ 