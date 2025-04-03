#include "dbc_parser.h"

#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <regex>
#include <iostream>

namespace dbc_parser {

// ========== DbcFile implementation ==========

DbcFile::DbcFile() : version_("") {}

DbcFile::~DbcFile() = default;

std::string DbcFile::GetVersion() const {
  return version_;
}

void DbcFile::SetVersion(const std::string& version) {
  version_ = version;
}

const std::vector<Node>& DbcFile::GetNodes() const {
  return nodes_;
}

void DbcFile::AddNode(const Node& node) {
  nodes_.push_back(node);
}

void DbcFile::AddNode(const std::string& name) {
  Node node;
  node.name = name;
  nodes_.push_back(node);
}

const std::vector<Message>& DbcFile::GetMessages() const {
  return messages_;
}

void DbcFile::AddMessage(const Message& message) {
  messages_.push_back(message);
}

// Helper method to get a mutable reference to the last message
Message& DbcFile::GetLastMessage() {
  return messages_.back();
}

// ========== DbcParser implementation ==========

DbcParser::DbcParser() = default;

DbcParser::~DbcParser() = default;

// Helper function to parse signal details
bool ParseSignalLine(const std::string& line, Signal* signal) {
  // Simple parsing - looking for specific patterns
  if (line.find(" SG_ ") != 0) {
    return false;
  }
  
  // Extract signal name
  size_t name_start = line.find(" SG_ ") + 5;
  size_t name_end = line.find(" :", name_start);
  if (name_end == std::string::npos) {
    return false;
  }
  signal->name = line.substr(name_start, name_end - name_start);
  
  // Extract start bit and length
  size_t bit_info_start = line.find(":", name_end) + 1;
  size_t bit_info_end = line.find("@", bit_info_start);
  if (bit_info_end == std::string::npos) {
    return false;
  }
  
  std::string bit_info = line.substr(bit_info_start, bit_info_end - bit_info_start);
  size_t separator = bit_info.find("|");
  if (separator == std::string::npos) {
    return false;
  }
  
  signal->start_bit = std::stoi(bit_info.substr(0, separator));
  signal->length = std::stoi(bit_info.substr(separator + 1));
  
  // Extract byte order and sign
  size_t byte_order_pos = bit_info_end + 1;
  if (byte_order_pos < line.length()) {
    // In DBC format:
    // - 0 = Intel (Little Endian)
    // - 1 = Motorola (Big Endian)
    signal->byte_order = (line[byte_order_pos] == '0') ? ByteOrder::kLittleEndian : ByteOrder::kBigEndian;
  }
  
  size_t sign_pos = byte_order_pos + 1;
  if (sign_pos < line.length()) {
    signal->is_signed = (line[sign_pos] == '-');
  }
  
  // Extract factor and offset
  size_t factor_start = line.find("(", sign_pos) + 1;
  size_t factor_end = line.find(",", factor_start);
  if (factor_start != std::string::npos && factor_end != std::string::npos) {
    signal->factor = std::stod(line.substr(factor_start, factor_end - factor_start));
    
    size_t offset_start = factor_end + 1;
    size_t offset_end = line.find(")", offset_start);
    if (offset_end != std::string::npos) {
      signal->offset = std::stod(line.substr(offset_start, offset_end - offset_start));
    }
  }
  
  // Extract min and max values
  size_t range_start = line.find("[", factor_start) + 1;
  size_t range_separator = line.find("|", range_start);
  size_t range_end = line.find("]", range_separator);
  if (range_start != std::string::npos && range_separator != std::string::npos && range_end != std::string::npos) {
    signal->min_value = std::stod(line.substr(range_start, range_separator - range_start));
    signal->max_value = std::stod(line.substr(range_separator + 1, range_end - range_separator - 1));
  }
  
  // Extract unit
  size_t unit_start = line.find("\"", range_end) + 1;
  size_t unit_end = line.find("\"", unit_start);
  if (unit_start != std::string::npos && unit_end != std::string::npos) {
    signal->unit = line.substr(unit_start, unit_end - unit_start);
  }
  
  // Extract receiver nodes
  size_t receivers_start = unit_end + 1;
  if (receivers_start < line.length()) {
    std::string receivers = line.substr(receivers_start);
    std::istringstream iss(receivers);
    std::string receiver;
    while (iss >> receiver) {
      // Handle comma-separated receivers
      if (receiver.back() == ',') {
        receiver.pop_back();
      }
      signal->receiver_nodes.push_back(receiver);
    }
  }
  
  return true;
}

int DbcParser::Parse(const std::string& file_path, std::unique_ptr<DbcFile>* dbc_file) {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    last_error_ = "Failed to open file: " + file_path;
    return 1;
  }

  // Create a new DbcFile object
  *dbc_file = std::make_unique<DbcFile>();

  // Parse file line by line
  std::string line;
  Message* current_message = nullptr;
  
  while (std::getline(file, line)) {
    // Parse VERSION
    if (line.find("VERSION") == 0) {
      std::regex version_regex(R"(VERSION\s+\"([^\"]+)\")");
      std::smatch match;
      if (std::regex_search(line, match, version_regex) && match.size() > 1) {
        (*dbc_file)->SetVersion(match[1].str());
      }
      continue;
    }
    
    // Parse Nodes (BU_)
    if (line.find("BU_:") == 0) {
      std::string nodes_line = line.substr(4); // Skip "BU_:"
      std::regex node_name_regex(R"((\w+))");
      
      auto nodes_begin = std::sregex_iterator(nodes_line.begin(), nodes_line.end(), node_name_regex);
      auto nodes_end = std::sregex_iterator();
      
      for (std::sregex_iterator i = nodes_begin; i != nodes_end; ++i) {
        std::smatch node_match = *i;
        (*dbc_file)->AddNode(node_match[1].str());
      }
      continue;
    }
    
    // Parse Message (BO_)
    if (line.find("BO_") == 0) {
      std::regex message_regex(R"(BO_\s+(\d+)\s+(\w+)\s*:\s*(\d+)\s+(\w+))");
      std::smatch msg_match;
      if (std::regex_search(line, msg_match, message_regex) && msg_match.size() > 4) {
        Message message;
        message.id = std::stoi(msg_match[1].str());
        message.name = msg_match[2].str();
        message.dlc = std::stoi(msg_match[3].str());
        message.sender = msg_match[4].str();
        
        (*dbc_file)->AddMessage(message);
        current_message = &(*dbc_file)->GetLastMessage();
      }
      continue;
    }
    
    // Parse Signal (SG_)
    if (line.find(" SG_") == 0 && current_message != nullptr) {
      Signal signal;
      if (ParseSignalLine(line, &signal)) {
        current_message->signals.push_back(signal);
      }
    }
  }
  
  return 0;  // Success
}

std::string DbcParser::GetLastError() const {
  return last_error_;
}

}  // namespace dbc_parser 