#include "dbc_parser/signal_decoder.h"

#include <cmath>
#include <stdexcept>
#include <algorithm>

namespace dbc_parser {

uint64_t SignalDecoder::extract_raw_value(const std::vector<uint8_t>& data,
                                         uint32_t start_bit,
                                         uint32_t length,
                                         bool is_little_endian,
                                         bool is_signed) {
  // Check if we have enough data
  uint32_t last_bit = is_little_endian 
      ? start_bit + length - 1
      : (start_bit / 8) * 8 + (7 - (start_bit % 8)) + length - 1;
  
  uint32_t last_byte = last_bit / 8;
  if (data.size() <= last_byte) {
    throw std::runtime_error("Not enough data to extract signal value");
  }
  
  uint64_t result = 0;
  
  if (is_little_endian) {
    // Little endian (Intel) format
    // For automotive CAN messages in little endian format:
    // - The start_bit indicates the LSB position
    // - Bits are read continuously from that position
    
    // This implementation matches the Intel format used in DBC files
    for (uint32_t i = 0; i < length; i++) {
      uint32_t byte_index = (start_bit + i) / 8;
      uint32_t bit_index = (start_bit + i) % 8;
      
      if (byte_index < data.size()) {
        if ((data[byte_index] >> bit_index) & 1) {
          result |= (1ULL << i);
        }
      }
    }
  } else {
    // Big endian (Motorola) format
    // For automotive CAN messages in big endian format:
    // - The start_bit indicates the MSB position
    // - Bit order is MSB to LSB within each byte
    // - Byte order is from MSB byte to LSB byte
    
    uint32_t byte_index = start_bit / 8;
    uint32_t bit_index = start_bit % 8;
    
    // In Motorola format, the start_bit is the position of the MSB
    // Bits are ordered from this MSB down to LSB within each byte,
    // then continue from the MSB of the next byte.
    
    for (uint32_t i = 0; i < length; i++) {
      uint32_t current_bit = 7 - bit_index - i % 8;
      uint32_t current_byte = byte_index + i / 8;
      
      if (current_byte < data.size()) {
        // Set the corresponding bit in the result
        if ((data[current_byte] >> current_bit) & 1) {
          result |= (1ULL << (length - 1 - i));
        }
      }
    }
  }
  
  // If this is a signed value and the highest bit is set, perform sign extension
  if (is_signed && (result & (1ULL << (length - 1)))) {
    result |= ~((1ULL << length) - 1);
  }
  
  return result;
}

double SignalDecoder::convert_to_physical(uint64_t raw_value,
                                         uint32_t length,
                                         bool is_signed,
                                         double factor,
                                         double offset) {
  // For signed values, convert to proper signed representation
  int64_t signed_raw_value;
  if (is_signed && (raw_value & (1ULL << (length - 1)))) {
    // This is a negative number, convert to proper signed value using twos complement
    signed_raw_value = twos_complement(raw_value, length);
  } else {
    // This is a positive number
    signed_raw_value = static_cast<int64_t>(raw_value);
  }
  
  // Apply factor and offset according to physical = raw * factor + offset
  return (static_cast<double>(signed_raw_value) * factor) + offset;
}

double SignalDecoder::decode(const std::vector<uint8_t>& data,
                            uint32_t start_bit,
                            uint32_t length,
                            bool is_little_endian,
                            bool is_signed,
                            double factor,
                            double offset) {
  // Extract the raw value and convert to physical value
  uint64_t raw_value = extract_raw_value(data, start_bit, length, is_little_endian, is_signed);
  return convert_to_physical(raw_value, length, is_signed, factor, offset);
}

uint64_t SignalDecoder::convert_from_physical(double physical_value,
                                             uint32_t length,
                                             bool is_signed,
                                             double factor,
                                             double offset) {
  // Convert physical value to raw value using physical = raw * factor + offset
  // So raw = (physical - offset) / factor
  double raw_double = (physical_value - offset) / factor;
  
  // Round to nearest integer
  int64_t raw_value;
  if (raw_double >= 0) {
    raw_value = static_cast<int64_t>(raw_double + 0.5);
  } else {
    raw_value = static_cast<int64_t>(raw_double - 0.5);
  }
  
  // Calculate the min/max values for this data type
  int64_t max_value = is_signed 
      ? (1LL << (length - 1)) - 1 
      : (1LL << length) - 1;
  int64_t min_value = is_signed 
      ? -(1LL << (length - 1)) 
      : 0;
  
  // Clamp to valid range
  raw_value = std::max(std::min(raw_value, max_value), min_value);
  
  // For signed negative values, convert to two's complement representation
  if (is_signed && raw_value < 0) {
    // Add 2^length to get the two's complement representation
    return static_cast<uint64_t>((1ULL << length) + raw_value);
  }
  
  return static_cast<uint64_t>(raw_value);
}

void SignalDecoder::encode(double physical_value,
                          std::vector<uint8_t>& data,
                          uint32_t start_bit,
                          uint32_t length,
                          bool is_little_endian,
                          bool is_signed,
                          double factor,
                          double offset) {
  // Convert physical value to raw value
  uint64_t raw_value = convert_from_physical(physical_value, length, is_signed, factor, offset);
  
  // Calculate maximum bit position to ensure data vector is large enough
  uint32_t last_bit = is_little_endian 
      ? start_bit + length - 1
      : (start_bit / 8) * 8 + (7 - (start_bit % 8)) + length - 1;
  
  uint32_t required_bytes = (last_bit / 8) + 1;
  if (data.size() < required_bytes) {
    data.resize(required_bytes, 0);
  }
  
  if (is_little_endian) {
    // Little endian (Intel) format
    for (uint32_t i = 0; i < length; i++) {
      uint32_t byte_index = (start_bit + i) / 8;
      uint32_t bit_index = (start_bit + i) % 8;
      
      if ((raw_value >> i) & 1) {
        data[byte_index] |= (1 << bit_index);
      } else {
        data[byte_index] &= ~(1 << bit_index);
      }
    }
  } else {
    // Big endian (Motorola) format
    uint32_t byte_index = start_bit / 8;
    uint32_t bit_index = start_bit % 8;
    
    for (uint32_t i = 0; i < length; i++) {
      uint32_t current_bit = 7 - bit_index - i % 8;
      uint32_t current_byte = byte_index + i / 8;
      
      if (current_byte < data.size()) {
        if ((raw_value >> (length - 1 - i)) & 1) {
          data[current_byte] |= (1 << current_bit);
        } else {
          data[current_byte] &= ~(1 << current_bit);
        }
      }
    }
  }
}

int64_t SignalDecoder::twos_complement(uint64_t value, uint32_t bit_length) {
  // If the sign bit is set (highest bit is 1)
  if (value & (1ULL << (bit_length - 1))) {
    // Calculate the negative value using two's complement
    return static_cast<int64_t>(value) - (1LL << bit_length);
  }
  
  // The value is positive, just return it as is
  return static_cast<int64_t>(value);
}

} // namespace dbc_parser 