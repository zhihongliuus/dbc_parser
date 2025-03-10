#include "dbc_parser/decoder.h"
#include "dbc_parser/parser.h"
#include "dbc_parser/types.h"

#include <boost/program_options.hpp>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace po = boost::program_options;

// Helper function to print message info
void print_message_info(const dbc_parser::Message& message) {
  std::cout << "Message: " << message.name() << " (ID: 0x" 
            << std::hex << std::setw(3) << std::setfill('0') << message.id() << std::dec
            << ", Length: " << message.length() << " bytes, Sender: " << message.sender() << ")\n";
            
  if (!message.comment().empty()) {
    std::cout << "  Comment: " << message.comment() << "\n";
  }
  
  std::cout << "  Signals:\n";
  for (const auto& sig_pair : message.signals()) {
    const dbc_parser::Signal& signal = *sig_pair.second;
    
    std::string mux_info;
    if (signal.mux_type() == dbc_parser::MultiplexerType::kMultiplexor) {
      mux_info = " [Multiplexor]";
    } else if (signal.mux_type() == dbc_parser::MultiplexerType::kMultiplexed) {
      mux_info = " [Multiplexed, mux_value=" + std::to_string(signal.mux_value()) + "]";
    }
    
    std::cout << "    " << signal.name() << mux_info << ": " 
              << "Start bit=" << signal.start_bit() 
              << ", Length=" << signal.length() 
              << ", " << (signal.is_little_endian() ? "Intel" : "Motorola")
              << ", " << (signal.is_signed() ? "Signed" : "Unsigned")
              << ", Factor=" << signal.factor()
              << ", Offset=" << signal.offset()
              << ", Range=[" << signal.min_value() << ", " << signal.max_value() << "]"
              << ", Unit=\"" << signal.unit() << "\""
              << "\n";
              
    if (!signal.comment().empty()) {
      std::cout << "      Comment: " << signal.comment() << "\n";
    }
    
    if (!signal.value_descriptions().empty()) {
      std::cout << "      Value descriptions:\n";
      for (const auto& val_pair : signal.value_descriptions()) {
        std::cout << "        " << val_pair.first << " = \"" << val_pair.second << "\"\n";
      }
    }
    
    if (!signal.receivers().empty()) {
      std::cout << "      Receivers: ";
      for (size_t i = 0; i < signal.receivers().size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << signal.receivers()[i];
      }
      std::cout << "\n";
    }
  }
  
  std::cout << "\n";
}

// Helper function to decode a CAN frame
void decode_frame(const dbc_parser::Database& db, uint32_t message_id, const std::string& hex_data) {
  // Convert hex string to bytes
  std::vector<uint8_t> data;
  for (size_t i = 0; i < hex_data.length(); i += 2) {
    std::string byte_str = hex_data.substr(i, 2);
    try {
      uint8_t byte = static_cast<uint8_t>(std::stoi(byte_str, nullptr, 16));
      data.push_back(byte);
    } catch (const std::exception& e) {
      std::cerr << "Error parsing hex data: " << e.what() << std::endl;
      return;
    }
  }
  
  // Create decoder
  dbc_parser::DecoderOptions options;
  options.verbose = true;
  dbc_parser::Decoder decoder(std::make_shared<dbc_parser::Database>(db), options);
  
  // Decode the frame
  auto decoded = decoder.decode_frame(message_id, data);
  if (!decoded) {
    std::cerr << "Failed to decode message with ID: 0x" 
              << std::hex << message_id << std::dec << std::endl;
    return;
  }
  
  // Print decoded data
  std::cout << "Decoded message: " << decoded->name << " (ID: 0x" 
            << std::hex << std::setw(3) << std::setfill('0') << decoded->id << std::dec << ")\n";
  std::cout << "Data: ";
  for (uint8_t byte : data) {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
  }
  std::cout << std::dec << "\n";
  
  std::cout << "Signals:\n";
  for (const auto& sig_pair : decoded->signals) {
    const dbc_parser::DecodedSignal& signal = sig_pair.second;
    std::cout << "  " << signal.name << " = " << signal.value;
    
    if (!signal.unit.empty()) {
      std::cout << " " << signal.unit;
    }
    
    if (!signal.description.empty()) {
      std::cout << " (" << signal.description << ")";
    }
    
    std::cout << "\n";
  }
}

int main(int argc, char* argv[]) {
  try {
    // Define command line options
    po::options_description desc("DBC Parser CLI Options");
    desc.add_options()
      ("help,h", "Show help message")
      ("input,i", po::value<std::string>(), "Input DBC or KCD file")
      ("list-messages,l", "List all messages in the DBC file")
      ("message,m", po::value<std::string>(), "Show details for a specific message (ID or name)")
      ("decode,d", po::value<std::vector<std::string>>()->multitoken(), "Decode a CAN frame: <ID> <hex data>")
      ("verbose,v", "Enable verbose output");
      
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    
    // Show help and exit if requested
    if (vm.count("help") || argc == 1) {
      std::cout << "DBC Parser CLI\n\n"
                << "A command-line tool for working with DBC and KCD CAN database files.\n\n"
                << desc << std::endl;
      return 0;
    }
    
    // Check for input file
    if (!vm.count("input")) {
      std::cerr << "Error: Input file is required." << std::endl;
      return 1;
    }
    
    // Load the DBC file
    std::string input_file = vm["input"].as<std::string>();
    dbc_parser::ParserOptions parser_options;
    parser_options.verbose = vm.count("verbose") > 0;
    
    std::unique_ptr<dbc_parser::Parser> parser = dbc_parser::ParserFactory::create_parser(input_file);
    std::unique_ptr<dbc_parser::Database> db = parser->parse_file(input_file, parser_options);
    
    if (!db) {
      std::cerr << "Error: Failed to parse file: " << input_file << std::endl;
      return 1;
    }
    
    std::cout << "Successfully parsed: " << input_file << "\n";
    
    // List all messages
    if (vm.count("list-messages")) {
      std::cout << "\nMessages:\n";
      for (const auto& msg_pair : db->messages()) {
        const dbc_parser::Message& message = *msg_pair.second;
        std::cout << "  " << message.name() << " (ID: 0x" 
                 << std::hex << std::setw(3) << std::setfill('0') << message.id() << std::dec
                 << ", " << message.signals().size() << " signals)\n";
      }
      std::cout << std::endl;
    }
    
    // Show details for a specific message
    if (vm.count("message")) {
      std::string message_id_or_name = vm["message"].as<std::string>();
      
      // Try to parse as ID first
      try {
        uint32_t id;
        if (message_id_or_name.substr(0, 2) == "0x") {
          id = std::stoul(message_id_or_name, nullptr, 16);
        } else {
          id = std::stoul(message_id_or_name);
        }
        
        const dbc_parser::Message* message = db->get_message(id);
        if (message) {
          print_message_info(*message);
        } else {
          std::cerr << "Error: Message with ID " << id << " not found." << std::endl;
        }
      } catch (const std::exception& e) {
        // Not a valid ID, try as name
        bool found = false;
        for (const auto& msg_pair : db->messages()) {
          if (msg_pair.second->name() == message_id_or_name) {
            print_message_info(*msg_pair.second);
            found = true;
            break;
          }
        }
        
        if (!found) {
          std::cerr << "Error: Message with name \"" << message_id_or_name << "\" not found." << std::endl;
        }
      }
    }
    
    // Decode a CAN frame
    if (vm.count("decode")) {
      std::vector<std::string> decode_args = vm["decode"].as<std::vector<std::string>>();
      if (decode_args.size() < 2) {
        std::cerr << "Error: Decode requires at least 2 arguments: <ID> <hex data>" << std::endl;
        return 1;
      }
      
      // Parse message ID
      uint32_t id;
      try {
        if (decode_args[0].substr(0, 2) == "0x") {
          id = std::stoul(decode_args[0], nullptr, 16);
        } else {
          id = std::stoul(decode_args[0]);
        }
      } catch (const std::exception& e) {
        std::cerr << "Error: Invalid message ID: " << decode_args[0] << std::endl;
        return 1;
      }
      
      // Decode the frame
      decode_frame(*db, id, decode_args[1]);
    }
    
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  
  return 0;
} 