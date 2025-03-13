#include "dbc_parser/decoder.h"
#include "dbc_parser/types.h"

#include <iostream>
#include <cmath>
#include <algorithm>

namespace dbc_parser {

/**
 * General-purpose implementation of the Decoder class that uses a Database to decode CAN frames.
 * This implementation relies entirely on the interfaces provided by Database, Message, and Signal.
 */
class Decoder::Impl {
public:
  Impl(const Database& db, const DecoderOptions& options)
    : db_(db), options_(options) {}
  
  std::optional<DecodedMessage> decode_frame(MessageId id, const std::vector<uint8_t>& data) const {
    // Find the message in the database
    const Message* message = db_.get_message(id);
    
    // If message not found and we're not ignoring unknown IDs, return nullopt
    if (!message && !options_.ignore_unknown_ids) {
      if (options_.verbose) {
        std::cerr << "Unknown message ID: " << id << std::endl;
      }
      return std::nullopt;
    }
    
    // If message not found but we're ignoring unknown IDs, return a placeholder
    if (!message) {
      DecodedMessage result;
      result.id = id;
      result.name = "UNKNOWN_" + std::to_string(id);
      return result;
    }
    
    // Check if any signal is out of bounds of the message's declared length
    for (const auto& [name, signal] : message->signals()) {
      uint32_t signal_last_bit = signal->start_bit() + signal->length();
      if (signal_last_bit > message->length() * 8) {
        if (options_.verbose) {
          std::cerr << "Signal " << signal->name() << " is out of bounds for message " << message->name()
                    << ": requires bit " << signal_last_bit - 1 << " but message length is "
                    << message->length() << " bytes (" << message->length() * 8 << " bits)" << std::endl;
        }
        return std::nullopt;
      }
    }
    
    DecodedMessage result;
    result.id = id;
    result.name = message->name();
    
    // First pass: find multiplexor value if any
    uint32_t mux_value = 0;
    bool has_multiplexor = false;
    
    // Check if we have enough data for any signal in the message
    bool has_valid_signals = false;
    
    for (const auto& [name, signal] : message->signals()) {
      uint32_t signal_last_byte = (signal->start_bit() + signal->length() + 7) / 8;
      if (data.size() >= signal_last_byte) {
        has_valid_signals = true;
        if (signal->mux_type() == MultiplexerType::kMultiplexor) {
          // Decode the multiplexor signal
          auto decoded_signal = decode_signal_internal(
            signal->start_bit(),
            signal->length(),
            signal->is_little_endian(),
            signal->is_signed(),
            signal->factor(),
            signal->offset(),
            data
          );
          
          mux_value = static_cast<uint32_t>(decoded_signal.value);
          has_multiplexor = true;
          
          // Add the multiplexor signal to the result
          DecodedSignal decoded;
          decoded.name = signal->name();
          decoded.value = decoded_signal.value;
          decoded.unit = signal->unit();
          
          // Check if there's a value description
          const auto& descriptions = signal->value_descriptions();
          auto it = descriptions.find(static_cast<int64_t>(decoded_signal.value));
          if (it != descriptions.end()) {
            decoded.description = it->second;
          }
          
          result.signals.emplace(signal->name(), std::move(decoded));
          break;
        }
      }
    }
    
    // If we don't have enough data for any signal, return nullopt
    if (!has_valid_signals) {
      if (options_.verbose) {
        std::cerr << "Data too short for any signal in message " << message->name()
                  << ": got " << data.size() << " bytes" << std::endl;
      }
      return std::nullopt;
    }
    
    // Second pass: decode all signals that have enough data
    for (const auto& [name, signal] : message->signals()) {
      // Skip the multiplexor signal (already decoded)
      if (signal->mux_type() == MultiplexerType::kMultiplexor) {
        continue;
      }
      
      // Skip multiplexed signals that don't match the current multiplexor value
      if (signal->mux_type() == MultiplexerType::kMultiplexed &&
          signal->mux_value() != mux_value) {
        continue;
      }
      
      // Check if we have enough data for this signal
      uint32_t signal_last_byte = (signal->start_bit() + signal->length() + 7) / 8;
      if (data.size() >= signal_last_byte) {
        // Decode the signal
        auto decoded_signal = decode_signal_internal(
          signal->start_bit(),
          signal->length(),
          signal->is_little_endian(),
          signal->is_signed(),
          signal->factor(),
          signal->offset(),
          data
        );
        
        DecodedSignal decoded;
        decoded.name = signal->name();
        decoded.value = decoded_signal.value;
        decoded.unit = signal->unit();
        
        // Check if there's a value description
        const auto& descriptions = signal->value_descriptions();
        auto it = descriptions.find(static_cast<int64_t>(decoded_signal.value));
        if (it != descriptions.end()) {
          decoded.description = it->second;
        }
        
        result.signals.emplace(signal->name(), std::move(decoded));
      } else if (options_.verbose) {
        std::cerr << "Skipping signal " << signal->name() << " in message " << message->name()
                  << ": requires " << signal_last_byte << " bytes, got " << data.size() << std::endl;
      }
    }
    
    return result;
  }
  
  std::optional<DecodedSignal> decode_signal(MessageId id, const std::string& signal_name,
                                           const std::vector<uint8_t>& data) const {
    // Find the message in the database
    const Message* message = db_.get_message(id);
    if (!message) {
      if (options_.verbose) {
        std::cerr << "Unknown message ID: " << id << std::endl;
      }
      return std::nullopt;
    }
    
    // Find the signal in the message
    const Signal* signal = message->get_signal(signal_name);
    if (!signal) {
      if (options_.verbose) {
        std::cerr << "Signal not found: " << signal_name << " in message "
                  << message->name() << std::endl;
      }
      return std::nullopt;
    }
    
    // Check if we have enough data for this signal
    uint32_t signal_last_byte = (signal->start_bit() + signal->length() + 7) / 8;
    if (data.size() < signal_last_byte) {
      if (options_.verbose) {
        std::cerr << "Data too short for signal " << signal_name << " in message " << message->name()
                  << ": requires " << signal_last_byte << " bytes, got " << data.size() << std::endl;
      }
      return std::nullopt;
    }
    
    // Handle multiplexed signals
    if (signal->mux_type() == MultiplexerType::kMultiplexed) {
      // Find the multiplexor signal
      for (const auto& [name, signal_ptr] : message->signals()) {
        if (signal_ptr->mux_type() == MultiplexerType::kMultiplexor) {
          // Check if we have enough data for the multiplexor signal
          uint32_t mux_last_byte = (signal_ptr->start_bit() + signal_ptr->length() + 7) / 8;
          if (data.size() < mux_last_byte) {
            if (options_.verbose) {
              std::cerr << "Data too short for multiplexor signal in message " << message->name()
                        << ": requires " << mux_last_byte << " bytes, got " << data.size() << std::endl;
            }
            return std::nullopt;
          }
          
          // Decode the multiplexor signal
          auto mux_decoded = decode_signal_internal(
            signal_ptr->start_bit(),
            signal_ptr->length(),
            signal_ptr->is_little_endian(),
            signal_ptr->is_signed(),
            signal_ptr->factor(),
            signal_ptr->offset(),
            data
          );
          
          // Check if the multiplexor value matches
          if (signal->mux_value() != static_cast<uint32_t>(mux_decoded.value)) {
            if (options_.verbose) {
              std::cerr << "Multiplexed signal " << signal_name << " requires multiplexor value "
                        << signal->mux_value() << " but multiplexor is "
                        << static_cast<uint32_t>(mux_decoded.value) << std::endl;
            }
            return std::nullopt;
          }
          break;
        }
      }
    }
    
    // Decode the signal
    auto decoded_signal = decode_signal_internal(
      signal->start_bit(),
      signal->length(),
      signal->is_little_endian(),
      signal->is_signed(),
      signal->factor(),
      signal->offset(),
      data
    );
    
    DecodedSignal result;
    result.name = signal_name;
    result.value = decoded_signal.value;
    result.unit = signal->unit();
    
    // Check if there's a value description
    const auto& descriptions = signal->value_descriptions();
    auto it = descriptions.find(static_cast<int64_t>(decoded_signal.value));
    if (it != descriptions.end()) {
      result.description = it->second;
    }
    
    return result;
  }
  
  std::optional<std::string> get_value_description(MessageId id, const std::string& signal_name,
                                                 double value) const {
    // Find the message in the database
    const Message* message = db_.get_message(id);
    if (!message) {
      return std::nullopt;
    }
    
    // Find the signal in the message
    const Signal* signal = message->get_signal(signal_name);
    if (!signal) {
      return std::nullopt;
    }
    
    // Check if there's a value description
    const auto& descriptions = signal->value_descriptions();
    auto it = descriptions.find(static_cast<int64_t>(value));
    if (it != descriptions.end()) {
      return it->second;
    }
    
    return std::nullopt;
  }
  
private:
  struct DecodedRawSignal {
    double value;
  };
  
  DecodedRawSignal decode_signal_internal(uint32_t start_bit, uint32_t length,
                                         bool is_little_endian, bool is_signed,
                                         double factor, double offset,
                                         const std::vector<uint8_t>& data) const {
    // Extract the raw value from the data
    uint64_t raw_value = 0;
    
    if (is_little_endian) {
      // Intel format (little endian)
      uint32_t byte_index = start_bit / 8;
      uint32_t bit_index = start_bit % 8;
      uint32_t bits_remaining = length;
      
      while (bits_remaining > 0 && byte_index < data.size()) {
        uint32_t bits_to_read = std::min(std::min(8 - bit_index, bits_remaining), 
                                       static_cast<uint32_t>((data.size() - byte_index) * 8 - bit_index));
        uint64_t mask = ((1ULL << bits_to_read) - 1) << bit_index;
        uint64_t bits = (data[byte_index] & mask) >> bit_index;
        
        raw_value |= bits << (length - bits_remaining);
        
        bits_remaining -= bits_to_read;
        byte_index++;
        bit_index = 0;
      }
    } else {
      // Motorola format (big endian)
      uint32_t byte_index = start_bit / 8;
      uint32_t bit_index = 7 - (start_bit % 8);
      uint32_t bits_remaining = length;
      
      while (bits_remaining > 0 && byte_index < data.size()) {
        uint32_t bits_to_read = std::min(std::min(bit_index + 1, bits_remaining),
                                       static_cast<uint32_t>((data.size() - byte_index) * 8));
        uint64_t mask = ((1ULL << bits_to_read) - 1) << (bit_index - bits_to_read + 1);
        uint64_t bits = (data[byte_index] & mask) >> (bit_index - bits_to_read + 1);
        
        raw_value |= bits << (length - bits_remaining);
        
        bits_remaining -= bits_to_read;
        byte_index++;
        bit_index = 7;
      }
    }
    
    // Apply sign extension if needed
    if (is_signed && (raw_value & (1ULL << (length - 1)))) {
      raw_value |= ~((1ULL << length) - 1);
    }
    
    // Apply factor and offset
    double physical_value = static_cast<double>(raw_value) * factor + offset;
    
    DecodedRawSignal result;
    result.value = physical_value;
    return result;
  }
  
  const Database& db_;
  DecoderOptions options_;
};

// Decoder implementation
Decoder::Decoder(const Database& db, const DecoderOptions& options)
  : impl_(std::make_unique<Impl>(db, options)) {}

Decoder::~Decoder() = default;

std::optional<DecodedMessage> Decoder::decode_frame(MessageId id, const std::vector<uint8_t>& data) const {
  return impl_->decode_frame(id, data);
}

std::optional<DecodedSignal> Decoder::decode_signal(MessageId id, const std::string& signal_name,
                                                  const std::vector<uint8_t>& data) const {
  return impl_->decode_signal(id, signal_name, data);
}

std::optional<std::string> Decoder::get_value_description(MessageId id, const std::string& signal_name,
                                                        double value) const {
  return impl_->get_value_description(id, signal_name, value);
}

} // namespace dbc_parser 