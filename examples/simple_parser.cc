#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <string>
#include <memory>
#include "../src/dbc_parser.h"

void PrintMessage(const dbc_parser::Message& message) {
  std::cout << "Message ID: 0x" << std::hex << std::setw(3) << std::setfill('0') 
            << message.id << std::dec << " (" << message.id << ")" << std::endl;
  std::cout << "  Name: " << message.name << std::endl;
  std::cout << "  DLC: " << message.dlc << std::endl;
  std::cout << "  Sender: " << message.sender << std::endl;
  
  if (!message.comment.empty()) {
    std::cout << "  Comment: " << message.comment << std::endl;
  }
  
  if (!message.signals.empty()) {
    std::cout << "  Signals:" << std::endl;
    for (const auto& signal : message.signals) {
      std::string byte_order_str;
      if (signal.byte_order == dbc_parser::ByteOrder::kLittleEndian) {
        byte_order_str = "Intel (Little Endian)";
      } else {
        byte_order_str = "Motorola (Big Endian)";
      }
      
      std::string multiplexing_info;
      if (signal.is_multiplexer) {
        multiplexing_info = " [Multiplexer]";
      } else if (signal.multiplexer_value >= 0) {
        multiplexing_info = " [Multiplexed: " + std::to_string(signal.multiplexer_value) + "]";
      }
      
      std::cout << "    " << signal.name << multiplexing_info
                << " (Start: " << signal.start_bit
                << ", Len: " << signal.length
                << ", Order: " << byte_order_str
                << ", Sign: " << (signal.is_signed ? "Signed" : "Unsigned")
                << ")" << std::endl;
                
      std::cout << "      Factor: " << signal.factor
                << ", Offset: " << signal.offset
                << ", Range: [" << signal.min_value << ", " << signal.max_value << "]"
                << ", Unit: " << signal.unit << std::endl;
                
      if (!signal.comment.empty()) {
        std::cout << "      Comment: " << signal.comment << std::endl;
      }
      
      if (!signal.value_descriptions.empty()) {
        std::cout << "      Value descriptions:" << std::endl;
        for (const auto& [value, description] : signal.value_descriptions) {
          std::cout << "        " << value << " = \"" << description << "\"" << std::endl;
        }
      }
      
      if (!signal.receiver_nodes.empty()) {
        std::cout << "      Receivers: ";
        for (size_t i = 0; i < signal.receiver_nodes.size(); ++i) {
          if (i > 0) std::cout << ", ";
          std::cout << signal.receiver_nodes[i];
        }
        std::cout << std::endl;
      }
    }
  }
  std::cout << std::endl;
}

void PrintNodes(const std::vector<dbc_parser::Node>& nodes) {
  std::cout << "Nodes: " << std::endl;
  for (const auto& node : nodes) {
    std::cout << "  " << node.name;
    if (!node.comment.empty()) {
      std::cout << " - " << node.comment;
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <dbc_file>" << std::endl;
    return 1;
  }

  dbc_parser::DbcParser parser;
  std::unique_ptr<dbc_parser::DbcFile> dbc_file;
  
  int result = parser.Parse(argv[1], &dbc_file);
  if (result != 0) {
    std::cerr << "Failed to parse DBC file: " << parser.GetLastError() << std::endl;
    return 1;
  }

  std::cout << "DBC Version: " << dbc_file->GetVersion() << std::endl << std::endl;
  
  PrintNodes(dbc_file->GetNodes());

  std::cout << "Messages: " << std::endl;
  for (const auto& message : dbc_file->GetMessages()) {
    PrintMessage(message);
  }

  return 0;
} 