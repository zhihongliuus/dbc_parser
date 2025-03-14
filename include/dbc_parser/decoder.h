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

// Decoder class for decoding CAN frames
class Decoder {
public:
  Decoder(const Database& db, const DecoderOptions& options = DecoderOptions());
  ~Decoder();

  // Decode a CAN frame
  std::optional<DecodedMessage> decode_frame(uint32_t id, const std::vector<uint8_t>& data) const;

  // Decode a specific signal from a CAN frame
  std::optional<DecodedSignal> decode_signal(uint32_t id, const std::string& signal_name, const std::vector<uint8_t>& data) const;

  // Get value description for a signal value
  std::optional<std::string> get_value_description(uint32_t id, const std::string& signal_name, int64_t value) const;

private:
  class Impl;
  const Database& db_;
  DecoderOptions options_;
  std::unique_ptr<Impl> impl_;

  // Helper methods
  double decode_signal_value(const Signal& signal, const std::vector<uint8_t>& data) const;
  bool is_multiplexed_signal_active(const Signal& signal, const std::vector<uint8_t>& data) const;
};

} // namespace dbc_parser

#endif // DBC_PARSER_DECODER_H_ 