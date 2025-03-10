#ifndef DBC_PARSER_PARSER_H_
#define DBC_PARSER_PARSER_H_

#include <memory>
#include <string>
#include <vector>

#include "dbc_parser/types.h"

namespace dbc_parser {

// Forward declarations
class ParserErrorHandler;

// Parser options
struct ParserOptions {
  bool verbose = false;
};

// Parser interface
class Parser {
 public:
  virtual ~Parser() = default;
  
  // Parse a file into a database
  virtual std::unique_ptr<Database> parse_file(const std::string& filename, 
                                             const ParserOptions& options = ParserOptions()) = 0;
  
  // Parse a string into a database
  virtual std::unique_ptr<Database> parse_string(const std::string& content,
                                               const ParserOptions& options = ParserOptions()) = 0;
  
  // Write a database to a file
  virtual bool write_file(const Database& db, const std::string& filename) = 0;
  
  // Convert a database to a string
  virtual std::string write_string(const Database& db) = 0;
};

// DBC format parser
class DbcParser : public Parser {
 public:
  DbcParser();
  ~DbcParser() override;
  
  // Parse a file into a database
  std::unique_ptr<Database> parse_file(const std::string& filename, 
                                     const ParserOptions& options = ParserOptions()) override;
  
  // Parse a string into a database
  std::unique_ptr<Database> parse_string(const std::string& content,
                                       const ParserOptions& options = ParserOptions()) override;
  
  // Write a database to a file
  bool write_file(const Database& db, const std::string& filename) override;
  
  // Convert a database to a string
  std::string write_string(const Database& db) override;
  
 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

// KCD format parser
class KcdParser : public Parser {
 public:
  KcdParser();
  ~KcdParser() override;
  
  // Parse a file into a database
  std::unique_ptr<Database> parse_file(const std::string& filename, 
                                     const ParserOptions& options = ParserOptions()) override;
  
  // Parse a string into a database
  std::unique_ptr<Database> parse_string(const std::string& content,
                                       const ParserOptions& options = ParserOptions()) override;
  
  // Write a database to a file
  bool write_file(const Database& db, const std::string& filename) override;
  
  // Convert a database to a string
  std::string write_string(const Database& db) override;
  
 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

// Factory to create the appropriate parser based on file extension
class ParserFactory {
 public:
  static std::unique_ptr<Parser> create_parser(const std::string& filename);
  
  // Create a specific parser type
  static std::unique_ptr<Parser> create_dbc_parser();
  static std::unique_ptr<Parser> create_kcd_parser();
};

// Parser error handler interface
class ParserErrorHandler {
 public:
  virtual ~ParserErrorHandler() = default;
  
  virtual void on_error(const std::string& message, int line, int column) = 0;
  virtual void on_warning(const std::string& message, int line, int column) = 0;
  virtual void on_info(const std::string& message, int line, int column) = 0;
};

// Default error handler that prints to stderr
class DefaultParserErrorHandler : public ParserErrorHandler {
 public:
  explicit DefaultParserErrorHandler(bool verbose = false) : verbose_(verbose) {}
  
  void on_error(const std::string& message, int line, int column) override;
  void on_warning(const std::string& message, int line, int column) override;
  void on_info(const std::string& message, int line, int column) override;
  
 private:
  bool verbose_;
};

} // namespace dbc_parser

#endif // DBC_PARSER_PARSER_H_ 