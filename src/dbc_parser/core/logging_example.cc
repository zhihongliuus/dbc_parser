#include <iostream>
#include <string>

#include "dbc_parser/core/logger.h"
#include "dbc_parser/core/log_macros.h"

using dbc_parser::core::Logger;

void demonstrate_logging() {
  DBC_LOG_TRACE("This is a trace message with a parameter: {}", 42);
  DBC_LOG_DEBUG("This is a debug message with multiple parameters: {} and {}", "string", 3.14);
  DBC_LOG_INFO("This is an info message");
  DBC_LOG_WARN("This is a warning message");
  DBC_LOG_ERROR("This is an error message about file: {}", "missing.dbc");
  DBC_LOG_CRITICAL("This is a critical message about error: {}", "Out of memory");
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
  DBC_LOG_INFO("Logging example started");
  
  // Demonstrate logging at different levels
  demonstrate_logging();
  
  // Log a shutdown message
  DBC_LOG_INFO("Logging example completed");
  
  // Shutdown logger
  Logger::Shutdown();
  
  return 0;
} 