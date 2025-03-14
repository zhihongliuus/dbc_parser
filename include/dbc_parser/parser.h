#ifndef DBC_PARSER_PARSER_H_
#define DBC_PARSER_PARSER_H_

#include <string>
#include <memory>
#include <vector>

#include "dbc_parser/types.h"

namespace dbc_parser {

// Forward declarations
class Database;
class ParserErrorHandler;

// DBC Parser class
class DbcParser {
public:
  DbcParser();
  ~DbcParser();

  // Parse methods
  std::unique_ptr<Database> parse_file(const std::string& filename, const ParserOptions& options);
  std::unique_ptr<Database> parse_string(const std::string& content, const ParserOptions& options);

  // Write methods
  bool write_file(const Database& db, const std::string& filename);
  std::string write_string(const Database& db);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

// Parser Factory class
class ParserFactory {
public:
  // Create a parser based on file extension
  static std::unique_ptr<DbcParser> create_parser(const std::string& filename);

  // Create a specific parser type
  static std::unique_ptr<DbcParser> create_dbc_parser();
};

} // namespace dbc_parser

#endif // DBC_PARSER_PARSER_H_
