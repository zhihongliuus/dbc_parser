#ifndef DBC_PARSER_CORE_LOG_MACROS_H_
#define DBC_PARSER_CORE_LOG_MACROS_H_

#include "dbc_parser/core/logger.h"

// Declare non-templated logging functions to avoid direct method calls on incomplete types
namespace dbc_parser {
namespace core {
namespace log {

// Basic logging functions with common format
void trace(const char* fmt, ...);
void debug(const char* fmt, ...);
void info(const char* fmt, ...);
void warn(const char* fmt, ...);
void error(const char* fmt, ...);
void critical(const char* fmt, ...);

// Simple string logging functions
void trace_str(const std::string& msg);
void debug_str(const std::string& msg);
void info_str(const std::string& msg);
void warn_str(const std::string& msg);
void error_str(const std::string& msg);
void critical_str(const std::string& msg);

} // namespace log
} // namespace core
} // namespace dbc_parser

// Define convenience macros for logging
#define DBC_LOG_TRACE(...) ::dbc_parser::core::log::trace(__VA_ARGS__)
#define DBC_LOG_DEBUG(...) ::dbc_parser::core::log::debug(__VA_ARGS__)
#define DBC_LOG_INFO(...) ::dbc_parser::core::log::info(__VA_ARGS__)
#define DBC_LOG_WARN(...) ::dbc_parser::core::log::warn(__VA_ARGS__)
#define DBC_LOG_ERROR(...) ::dbc_parser::core::log::error(__VA_ARGS__)
#define DBC_LOG_CRITICAL(...) ::dbc_parser::core::log::critical(__VA_ARGS__)

// String-only versions for simple messages
#define DBC_LOG_TRACE_STR(msg) ::dbc_parser::core::log::trace_str(msg)
#define DBC_LOG_DEBUG_STR(msg) ::dbc_parser::core::log::debug_str(msg)
#define DBC_LOG_INFO_STR(msg) ::dbc_parser::core::log::info_str(msg)
#define DBC_LOG_WARN_STR(msg) ::dbc_parser::core::log::warn_str(msg)
#define DBC_LOG_ERROR_STR(msg) ::dbc_parser::core::log::error_str(msg)
#define DBC_LOG_CRITICAL_STR(msg) ::dbc_parser::core::log::critical_str(msg)

#endif  // DBC_PARSER_CORE_LOG_MACROS_H_ 