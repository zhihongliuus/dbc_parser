#ifndef DBC_PARSER_H_
#define DBC_PARSER_H_

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace dbc_parser {

// Class representing a parsed DBC file
class DbcFile {
 public:
  DbcFile();
  ~DbcFile();

  // Version information
  std::string GetVersion() const;
  void SetVersion(const std::string& version);

 private:
  std::string version_;
};

// Main parser class that handles DBC file parsing
class DbcParser {
 public:
  DbcParser();
  ~DbcParser();

  // Parse a DBC file and return a status code
  // 0 means success, other values indicate specific errors
  int Parse(const std::string& file_path, std::unique_ptr<DbcFile>* dbc_file);

  // Get the error message if parsing failed
  std::string GetLastError() const;

 private:
  std::string last_error_;
};

}  // namespace dbc_parser

#endif  // DBC_PARSER_H_ 