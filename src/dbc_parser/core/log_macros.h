#ifndef DBC_PARSER_CORE_LOG_MACROS_H_
#define DBC_PARSER_CORE_LOG_MACROS_H_

#include "dbc_parser/core/logger.h"
#include "spdlog/spdlog.h"

// Define convenience macros for logging
#define DBC_LOG_TRACE(...) if (::dbc_parser::core::Logger::GetLogger()) ::dbc_parser::core::Logger::GetLogger()->trace(__VA_ARGS__)
#define DBC_LOG_DEBUG(...) if (::dbc_parser::core::Logger::GetLogger()) ::dbc_parser::core::Logger::GetLogger()->debug(__VA_ARGS__)
#define DBC_LOG_INFO(...) if (::dbc_parser::core::Logger::GetLogger()) ::dbc_parser::core::Logger::GetLogger()->info(__VA_ARGS__)
#define DBC_LOG_WARN(...) if (::dbc_parser::core::Logger::GetLogger()) ::dbc_parser::core::Logger::GetLogger()->warn(__VA_ARGS__)
#define DBC_LOG_ERROR(...) if (::dbc_parser::core::Logger::GetLogger()) ::dbc_parser::core::Logger::GetLogger()->error(__VA_ARGS__)
#define DBC_LOG_CRITICAL(...) if (::dbc_parser::core::Logger::GetLogger()) ::dbc_parser::core::Logger::GetLogger()->critical(__VA_ARGS__)

#endif  // DBC_PARSER_CORE_LOG_MACROS_H_ 