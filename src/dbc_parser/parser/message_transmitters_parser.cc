#include "src/dbc_parser/parser/message_transmitters_parser.h"

#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace dbc_parser {
namespace parser {

std::optional<MessageTransmitters> MessageTransmittersParser::Parse(std::string_view input) {
  if (input.empty()) {
    return std::nullopt;
  }

  std::string input_str(input);
  std::istringstream iss(input_str);
  std::string token;
  
  // Parse BO_TX_BU_ keyword
  iss >> token;
  if (token != "BO_TX_BU_") {
    return std::nullopt;
  }
  
  // Parse message ID
  iss >> token;
  int message_id;
  try {
    message_id = std::stoi(token);
  } catch (...) {
    return std::nullopt;
  }
  
  // Parse colon
  iss >> token;
  if (token != ":") {
    return std::nullopt;
  }
  
  // Create result object
  MessageTransmitters result;
  result.message_id = message_id;
  
  // Parse transmitters (rest of the line)
  std::getline(iss, token);
  if (token.empty()) {
    return result;  // No transmitters
  }
  
  // Split the transmitter list by commas
  std::istringstream transmitter_stream(token);
  std::string transmitter;
  while (std::getline(transmitter_stream, transmitter, ',')) {
    // Trim leading/trailing whitespace
    transmitter.erase(0, transmitter.find_first_not_of(" \t\n\r\f\v"));
    transmitter.erase(transmitter.find_last_not_of(" \t\n\r\f\v") + 1);
    
    if (!transmitter.empty()) {
      result.transmitters.push_back(transmitter);
    }
  }
  
  return result;
}

}  // namespace parser
}  // namespace dbc_parser 