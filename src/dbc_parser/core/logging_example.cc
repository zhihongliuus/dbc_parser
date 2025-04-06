#include <iostream>
#include <string>
#include <sstream>

#include "dbc_parser/core/logger.h"
#include "dbc_parser/core/log_macros.h"

using dbc_parser::core::Logger;

void demonstrate_logging() {
  // Create formatted strings manually for now
  std::stringstream trace_msg;
  trace_msg << "This is a trace message with a parameter: " << 42;
  DBC_LOG_TRACE_STR(trace_msg.str());
  
  std::stringstream debug_msg;
  debug_msg << "This is a debug message with multiple parameters: string and " << 3.14;
  DBC_LOG_DEBUG_STR(debug_msg.str());
  
  DBC_LOG_INFO_STR("This is an info message");
  DBC_LOG_WARN_STR("This is a warning message");
  
  std::stringstream error_msg;
  error_msg << "This is an error message about file: missing.dbc";
  DBC_LOG_ERROR_STR(error_msg.str());
  
  std::stringstream critical_msg;
  critical_msg << "This is a critical message about error: Out of memory";
  DBC_LOG_CRITICAL_STR(critical_msg.str());
}

int main(int argc, char* argv[]) {
  std::string log_level = "debug";
  
  // Parse command line arguments
  if (argc > 1) {
    log_level = argv[1];
  }
  
  std::cout << "Initializing logger with level: " << log_level << std::endl;
  
  // Initialize logger
  if (!Logger::Initialize(log_level)) {
    std::cerr << "Failed to initialize logger" << std::endl;
    return 1;
  }
  
  // Log a startup message
  DBC_LOG_INFO_STR("Logging example started");
  
  // Demonstrate logging at different levels
  demonstrate_logging();
  
  // Log a shutdown message
  DBC_LOG_INFO_STR("Logging example completed");
  
  // Shutdown logger
  Logger::Shutdown();
  
  return 0;
} 