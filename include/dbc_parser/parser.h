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

struct ParserOptions {
  bool verbose = false;
  bool ignore_unknown_tags = true;
  bool strict_compliance = false;
};

class DbcParser {
public:
  DbcParser();
  ~DbcParser();

  std::unique_ptr<Database> parse_file(const std::string& filename, const ParserOptions& options = ParserOptions());
  std::unique_ptr<Database> parse_string(const std::string& content, const ParserOptions& options = ParserOptions());

  bool write_file(const Database& db, const std::string& filename);
  std::string write_string(const Database& db);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

// Factory for creating parsers based on file extension
class ParserFactory {
public:
  static std::unique_ptr<DbcParser> create_parser(const std::string& filename);
  static std::unique_ptr<DbcParser> create_dbc_parser();
};

} // namespace dbc_parser

#endif // DBC_PARSER_PARSER_H_
