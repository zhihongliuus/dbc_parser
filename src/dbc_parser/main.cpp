#include "dbc_parser/parser.h"
#include "dbc_parser/types.h"

#include <iostream>
#include <memory>
#include <string>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <dbc_file>" << std::endl;
    return 1;
  }

  try {
    dbc_parser::DbcParser parser;
    auto database = parser.parse_file(argv[1], {});

    // Print out the parsed data
    std::cout << "Successfully parsed DBC file!" << std::endl;

    // Print version
    if (database->version()) {
      std::cout << "Version: " << database->version()->version << std::endl;
    }

    // Print nodes
    std::cout << "Nodes:" << std::endl;
    for (const auto &node : database->nodes()) {
      std::cout << "  " << node->name();
      if (!node->comment().empty()) {
        std::cout << " (" << node->comment() << ")";
      }
      std::cout << std::endl;
    }

    // Print messages
    std::cout << "Messages:" << std::endl;
    for (const auto &message : database->messages()) {
      std::cout << "  " << message->name() << " (ID: " << message->id()
                << ", Length: " << message->length() << ")";
      if (!message->comment().empty()) {
        std::cout << " (" << message->comment() << ")";
      }
      std::cout << std::endl;

      // Print signals
      for (const auto &[name, signal] : message->signals()) {
        std::cout << "    " << signal->name() << " (" << signal->start_bit()
                  << "|" << signal->length() << ")";
        if (!signal->comment().empty()) {
          std::cout << " (" << signal->comment() << ")";
        }
        std::cout << std::endl;
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}