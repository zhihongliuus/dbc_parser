#include "dbc_parser/core/logger.h"

#include <iostream>
#include <memory>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace dbc_parser {
namespace core {

std::shared_ptr<spdlog::logger> Logger::logger_ = nullptr;

// Convert string log level to spdlog::level::level_enum
spdlog::level::level_enum StringToLogLevel(const std::string& level) {
  if (level == "trace") return spdlog::level::trace;
  if (level == "debug") return spdlog::level::debug;
  if (level == "info") return spdlog::level::info;
  if (level == "warn") return spdlog::level::warn;
  if (level == "error") return spdlog::level::err;
  if (level == "critical") return spdlog::level::critical;
  if (level == "off") return spdlog::level::off;
  
  // Default to info level
  return spdlog::level::info;
}

bool Logger::Initialize(const std::string& log_level) {
  try {
    // Create a color console sink
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(StringToLogLevel(log_level));
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
    
    // Create logger with sink
    logger_ = std::make_shared<spdlog::logger>("dbc_parser", console_sink);
    logger_->set_level(StringToLogLevel(log_level));
    
    // Register logger
    spdlog::register_logger(logger_);
    
    // Log initialization
    logger_->info("Logger initialized with level: {}", log_level);
    
    return true;
  } catch (const spdlog::spdlog_ex& ex) {
    std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
    return false;
  }
}

void Logger::Shutdown() {
  if (logger_) {
    logger_->flush();
    spdlog::shutdown();
    logger_ = nullptr;
  }
}

std::shared_ptr<spdlog::logger> Logger::GetLogger() {
  return logger_;
}

}  // namespace core
}  // namespace dbc_parser 