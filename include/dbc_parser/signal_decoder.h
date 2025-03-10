#ifndef DBC_PARSER_SIGNAL_DECODER_H_
#define DBC_PARSER_SIGNAL_DECODER_H_

#include <cstdint>
#include <vector>
#include <string>

namespace dbc_parser {

// Signal decoder class for extracting bits and converting values
class SignalDecoder {
 public:
  // Extract raw value from data
  static uint64_t extract_raw_value(const std::vector<uint8_t>& data,
                                   uint32_t start_bit,
                                   uint32_t length,
                                   bool is_little_endian,
                                   bool is_signed);
  
  // Convert raw value to physical value
  static double convert_to_physical(uint64_t raw_value,
                                   uint32_t length,
                                   bool is_signed,
                                   double factor,
                                   double offset);
  
  // Decode physical value from data
  static double decode(const std::vector<uint8_t>& data,
                      uint32_t start_bit,
                      uint32_t length,
                      bool is_little_endian,
                      bool is_signed,
                      double factor,
                      double offset);
  
  // Encode physical value to data
  static void encode(double physical_value,
                    std::vector<uint8_t>& data,
                    uint32_t start_bit,
                    uint32_t length,
                    bool is_little_endian,
                    bool is_signed,
                    double factor,
                    double offset);
                    
  // Convert from physical value to raw value
  static uint64_t convert_from_physical(double physical_value,
                                       uint32_t length,
                                       bool is_signed,
                                       double factor,
                                       double offset);
  
  // Twos complement calculation
  static int64_t twos_complement(uint64_t value, uint32_t bit_length);
};

} // namespace dbc_parser

#endif // DBC_PARSER_SIGNAL_DECODER_H_ 