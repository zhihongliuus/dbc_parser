# DBC Parser

A high-performance C++ library for parsing, manipulating, and decoding CAN DBC files.

## Features

- Fast parsing and decoding of DBC files using Boost Spirit
- Support for KCD file format
- Editing DBC/KCD through C++ interface
- Decoding functionality for CAN frames of arbitrary byte length
- cantools-like decoding interface
- Detailed error reporting for parser issues
- Comprehensive coverage of DBC data types:
  - Version
  - New symbols
  - Bit timing
  - Nodes
  - Value tables
  - Messages and signals
  - Signal multiplexing
  - Message transmitters
  - Environment variables
  - Signal types
  - Comments
  - Attribute definitions and values
  - Value descriptions
  - Signal extended value types
  - Signal groups

## Dependencies

- C++17 compatible compiler
- Boost (Spirit, Fusion, Phoenix, Program Options, Filesystem)
- Bazel build system

## Building

The project uses Bazel with bzlmod for dependency management.

```bash
# Build the library and CLI tool
bazel build //...

# Build with optimizations
bazel build -c opt //...

# Run tests
bazel test //...
```

## Usage Examples

### Command-line Interface

The project includes a command-line tool for basic DBC file operations:

```bash
# Show help
bazel run //:dbc_parser_cli -- --help

# List all messages in a DBC file
bazel run //:dbc_parser_cli -- -i my_file.dbc --list-messages

# Show details for a specific message
bazel run //:dbc_parser_cli -- -i my_file.dbc --message 0x123
bazel run //:dbc_parser_cli -- -i my_file.dbc --message EngineStatus

# Decode a CAN frame
bazel run //:dbc_parser_cli -- -i my_file.dbc --decode 0x123 1122334455667788
```

### C++ API

```cpp
#include "dbc_parser/parser.h"
#include "dbc_parser/decoder.h"

// Parse a DBC file
dbc_parser::ParserOptions options;
options.verbose = true;
auto parser = dbc_parser::ParserFactory::create_parser("my_file.dbc");
auto db = parser->parse_file("my_file.dbc", options);

// Access message and signal information
for (const auto& msg_pair : db->messages()) {
  const dbc_parser::Message& msg = *msg_pair.second;
  std::cout << "Message: " << msg.name() << " (ID: " << msg.id() << ")\n";
  
  for (const auto& sig_pair : msg.signals()) {
    const dbc_parser::Signal& sig = *sig_pair.second;
    std::cout << "  Signal: " << sig.name() << "\n";
  }
}

// Decode a CAN frame
std::vector<uint8_t> data = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
dbc_parser::Decoder decoder(std::make_shared<dbc_parser::Database>(*db));
auto decoded = decoder.decode_frame(0x123, data);

if (decoded) {
  for (const auto& sig_pair : decoded->signals) {
    std::cout << sig_pair.first << " = " << sig_pair.second.value << " " 
              << sig_pair.second.unit << "\n";
  }
}
```

## License

MIT

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. 