# DBC Parser

A modern C++ parser for DBC (CAN database) files using PEGTL.

## Features

- Modern C++17 implementation
- Comprehensive test suite
- Bazel build system
- Uses PEGTL for efficient parsing

## Dependencies

- Bazel 6.0+
- C++17 compatible compiler
- googletest (for testing)
- PEGTL (Parsing Expression Grammar Template Library)

## Project Structure

- `src/` - Source code
  - `core/` - Core utilities
  - `parser/` - DBC file parser implementation
- `tests/` - Test code
- `bazel/` - Build system configuration

## Building

```shell
bazel build //...
```

## Running Tests

```shell
bazel test //...
```

## Bzlmod Migration Status

This project is in the process of migrating from WORKSPACE to Bzlmod for dependency management.

### Current Status
- Dependencies are declared in both WORKSPACE and MODULE.bazel files
- BUILD files have been updated to use modern repository names (@googletest)
- The project builds and all tests pass
- The WORKSPACE file is still needed for compatibility

### Challenges
When attempting to build without the WORKSPACE file, we encounter repository resolution issues. 
Further investigation is needed to complete the migration.

### Migration Tools
A migration tool is provided to update repository references in BUILD files:

```shell
# Preview the changes
./migration_tool.py --preview

# Apply the changes
./migration_tool.py
```

## License

[Specify your license here] 