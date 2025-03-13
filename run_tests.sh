#!/bin/bash

# Run all tests with coverage
bazel coverage --combined_report=lcov //test/dbc_parser/...

# Run tests with debug mode
bazel test //:dbc_parser_test --compilation_mode=dbg

# Generate HTML coverage report (requires lcov)
genhtml "$(bazel info output_path)/_coverage/_coverage_report.dat" -o coverage_report

echo "Coverage report generated in coverage_report directory"
echo "Open coverage_report/index.html in a browser to view" 