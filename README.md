# DBC Parser

A robust, C++ library for parsing DBC files used in automotive CAN networks.

## Overview

This library provides a complete parser for DBC (Database CAN) files, which are widely used to describe CAN networks in automotive applications. The parser supports all elements of the DBC file format including messages, signals, value tables, and more.

## Features

- Complete DBC file format support
- Modern C++ implementation (C++17)
- Built with PEGTL (Parsing Expression Grammar Template Library)
- Robust error handling
- Well-tested using TDD methodology
- Easy-to-use API

## Supported DBC Elements

- Version Information
- New Symbols
- Bit Timing
- Nodes (ECUs)
- Value Tables
- Messages and Signals
- Signal Multiplexing
- Message Transmitters
- Environment Variables
- Signal Types
- Comments
- Attribute Definitions and Values
- Value Descriptions
- Signal Extended Value Types
- Signal Groups

## Building

This project uses Bazel as its build system.

### Prerequisites

- Bazel build system
- C++17 compatible compiler

### Build Steps

```bash
# Clone the repository
git clone https://github.com/yourusername/dbc_parser.git
cd dbc_parser

# Build the library
bazel build //:dbc_parser

# Build the examples
bazel build //examples:simple_parser

# Run the tests
bazel test //test/unit:dbc_parser_test
bazel test //test/integration:dbc_parser_integration_test
```

## Usage Example

```cpp
#include <iostream>
#include <memory>
#include "src/dbc_parser.h"

int main() {
  // Create a parser
  dbc_parser::DbcParser parser;
  
  // Parse a DBC file
  std::unique_ptr<dbc_parser::DbcFile> dbc_file;
  int result = parser.Parse("path/to/your/file.dbc", &dbc_file);
  
  if (result != 0) {
    std::cerr << "Error parsing DBC file: " << parser.GetLastError() << std::endl;
    return 1;
  }
  
  // Access DBC elements
  std::cout << "DBC Version: " << dbc_file->GetVersion() << std::endl;
  
  // More functionality will be added as the implementation progresses
  
  return 0;
}
```

## Development

This project follows Test-Driven Development principles:

1. Write a test that defines a function or improvements of a function
2. Run the test, which should fail because the function is not implemented
3. Write the simplest code that passes the test
4. Refactor the code until it conforms to the standards while passing the test

## License

This project is licensed under the MIT License - see the LICENSE file for details. 