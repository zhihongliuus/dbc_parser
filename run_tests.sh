#!/bin/bash

# Exit on any error
set -e

# Display the commands being executed
set -x

# Clean the build output to avoid stale dependencies
bazel clean

# Build and run tests with Bzlmod
bazel test //... --enable_bzlmod=true

echo "--------------------------------------------------------------"
echo "All tests have been successfully run"
echo "--------------------------------------------------------------"

# Generate coverage report
bazel coverage --combined_report=lcov //test/dbc_parser/...

# Generate HTML report
genhtml "$(bazel info output_path)/_coverage/_coverage_report.dat" -o coverage_report

echo "Test results and coverage report generated:"
echo " - Test output: bazel-testlogs/"
echo " - Coverage report: coverage_report/index.html" 