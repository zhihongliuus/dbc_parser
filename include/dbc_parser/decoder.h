#ifndef DBC_PARSER_DECODER_H_
#define DBC_PARSER_DECODER_H_

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <optional>
#include <cstdint>

#include "dbc_parser/types.h"

namespace dbc_parser {

// Forward declarations
class Database;

// Decoded signal structure
struct DecodedSignal {
  std::string name;
  double value;
  std::string unit;
  std::optional<std::string> description;
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

// Decoder class for decoding CAN frames
class Decoder {
public:
  explicit Decoder(const Database& db, const DecoderOptions& options = DecoderOptions());
  ~Decoder();

  // Decode a complete CAN frame
  std::optional<DecodedMessage> decode_frame(MessageId id, const std::vector<uint8_t>& data) const;
  
  // Decode a specific signal from a CAN frame
  std::optional<DecodedSignal> decode_signal(MessageId id, const std::string& signal_name,
                                            const std::vector<uint8_t>& data) const;
  
  // Get the textual description for a signal value
  std::optional<std::string> get_value_description(MessageId id, const std::string& signal_name,
                                                 double value) const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace dbc_parser

#endif // DBC_PARSER_DECODER_H_ 