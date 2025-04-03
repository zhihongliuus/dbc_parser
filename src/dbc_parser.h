#ifndef DBC_PARSER_H_
#define DBC_PARSER_H_

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace dbc_parser {

// Structure for a Node (ECU)
struct Node {
  std::string name;
  std::string comment;
};

// Enum for byte order
enum class ByteOrder {
  kLittleEndian,
  kBigEndian
};

// Structure for a Signal
struct Signal {
  std::string name;
  int start_bit = 0;
  int length = 0;
  ByteOrder byte_order = ByteOrder::kLittleEndian;
  bool is_signed = false;
  double factor = 1.0;
  double offset = 0.0;
  double min_value = 0.0;
  double max_value = 0.0;
  std::string unit;
  std::vector<std::string> receiver_nodes;
  std::string comment;
  std::unordered_map<int, std::string> value_descriptions;
};

// Structure for a Message
struct Message {
  int id = 0;
  std::string name;
  int dlc = 0;  // Data Length Code
  std::string sender;
  std::string comment;
  std::vector<Signal> signals;
};

// Class representing a parsed DBC file
class DbcFile {
 public:
  DbcFile();
  ~DbcFile();

  // Version information
  std::string GetVersion() const;
  void SetVersion(const std::string& version);

  // Nodes (ECUs)
  const std::vector<Node>& GetNodes() const;
  void AddNode(const Node& node);
  void AddNode(const std::string& name);
  
  // Messages
  const std::vector<Message>& GetMessages() const;
  void AddMessage(const Message& message);
  Message& GetLastMessage();
  
 private:
  std::string version_;
  std::vector<Node> nodes_;
  std::vector<Message> messages_;
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