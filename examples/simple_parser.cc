#include <iostream>
#include <memory>
#include <string>

#include "src/dbc_parser.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <dbc_file_path>" << std::endl;
    return 1;
  }

  std::string dbc_file_path = argv[1];
  dbc_parser::DbcParser parser;
  std::unique_ptr<dbc_parser::DbcFile> dbc_file;

  int result = parser.Parse(dbc_file_path, &dbc_file);
  if (result != 0) {
    std::cerr << "Error parsing DBC file: " << parser.GetLastError() << std::endl;
    return 1;
  }

  std::cout << "Successfully parsed DBC file: " << dbc_file_path << std::endl;
  std::cout << "Version: " << dbc_file->GetVersion() << std::endl;

  // TODO: Add more output as more functionality is implemented

  return 0;
} 