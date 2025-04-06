#ifndef DBC_PARSER_CORE_LOGGER_H_
#define DBC_PARSER_CORE_LOGGER_H_

#include <memory>
#include <string>

// Forward declare spdlog classes to avoid inclusion in header
namespace spdlog {
class logger;
}

namespace dbc_parser {
namespace core {

/**
 * @brief Logger class providing centralized logging functionality for the DBC parser.
 *
 * This class provides a thin wrapper around spdlog to standardize logging
 * throughout the DBC parser codebase.
 */
class Logger {
 public:
  /**
   * @brief Initializes the logger system.
   * 
   * Must be called before any logging operations to configure the logging system.
   * Sets up console logging with the specified log level.
   *
   * @param log_level Minimum log level for output ("trace", "debug", "info", "warn", "error", "critical")
   * @return true if initialization successful, false otherwise
   */
  static bool Initialize(const std::string& log_level = "info");

  /**
   * @brief Shuts down the logging system.
   * 
   * Flushes all pending logs and cleans up resources.
   */
  static void Shutdown();

  /**
   * @brief Gets the underlying spdlog logger.
   * 
   * @return The spdlog logger instance
   */
  static std::shared_ptr<spdlog::logger> GetLogger();

 private:
  static std::shared_ptr<spdlog::logger> logger_;
};

}  // namespace core
}  // namespace dbc_parser

#endif  // DBC_PARSER_CORE_LOGGER_H_ 