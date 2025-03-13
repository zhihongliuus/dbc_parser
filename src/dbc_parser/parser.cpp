#include "dbc_parser/parser.h"
#include "dbc_parser/dbc_grammar.h"
#include "dbc_parser/types.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <string>

namespace dbc_parser {

class DbcParser::Impl {
public:
  Impl() = default;
  
  std::unique_ptr<Database> parse_file(const std::string& filename, const ParserOptions& options) {
    // Check if file exists
    if (!std::filesystem::exists(filename)) {
      throw std::runtime_error("File not found: " + filename);
    }
    
    // Read file content
    std::ifstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("Failed to open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return parse_string(buffer.str(), options);
  }
  
  std::unique_ptr<Database> parse_string(const std::string& content, const ParserOptions& options) {
    if (content.empty()) {
      throw std::runtime_error("Empty DBC content");
    }
    
    auto db = std::make_unique<Database>();
    
    // Parse version
    size_t version_pos = content.find("VERSION \"");
    if (version_pos != std::string::npos) {
      size_t version_end = content.find("\"", version_pos + 9);
      if (version_end != std::string::npos) {
        Database::Version version;
        version.version = content.substr(version_pos + 9, version_end - (version_pos + 9));
        db->set_version(version);
      }
    }
    
    // Parse nodes
    size_t nodes_pos = content.find("BU_:");
    if (nodes_pos != std::string::npos) {
      size_t nodes_end = content.find("\n", nodes_pos);
      if (nodes_end != std::string::npos) {
        std::string nodes_line = content.substr(nodes_pos + 4, nodes_end - (nodes_pos + 4));
        std::istringstream nodes_stream(nodes_line);
        std::string node_name;
        while (nodes_stream >> node_name) {
          auto node = std::make_unique<Node>(node_name);
          db->add_node(std::move(node));
        }
      }
    }
    
    // Parse messages and signals
    size_t pos = 0;
    while ((pos = content.find("BO_", pos)) != std::string::npos) {
      size_t line_end = content.find("\n", pos);
      if (line_end == std::string::npos) break;
      
      std::string line = content.substr(pos, line_end - pos);
      std::istringstream line_stream(line);
      
      std::string bo_tag;
      MessageId id;
      std::string name;
      uint32_t length;
      std::string sender;
      
      line_stream >> bo_tag >> id >> name;
      if (name.back() == ':') {
        name.pop_back();
      }
      line_stream >> length >> sender;
      
      auto message = std::make_unique<Message>(id, name, length, sender);
      
      // Parse signals for this message
      size_t signal_pos = line_end + 1;
      while (signal_pos < content.length()) {
        size_t next_line = content.find("\n", signal_pos);
        if (next_line == std::string::npos) break;
        
        std::string signal_line = content.substr(signal_pos, next_line - signal_pos);
        if (signal_line.find(" SG_ ") == std::string::npos) break;
        
        std::istringstream signal_stream(signal_line);
        std::string sg_tag, signal_name, colon;
        uint32_t start_bit, length;
        std::string byte_order, value_type;
        double factor, offset, min_value, max_value;
        std::string unit;
        
        signal_stream >> sg_tag >> signal_name >> colon;
        
        // Parse signal format: start_bit|length@byte_order+/-
        std::string format;
        signal_stream >> format;
        size_t bar_pos = format.find("|");
        size_t at_pos = format.find("@");
        if (bar_pos != std::string::npos && at_pos != std::string::npos) {
          start_bit = std::stoul(format.substr(0, bar_pos));
          length = std::stoul(format.substr(bar_pos + 1, at_pos - (bar_pos + 1)));
          byte_order = format.substr(at_pos + 1, 1);
          value_type = format.substr(at_pos + 2, 1);
        }
        
        // Parse factor and offset
        std::string factor_offset;
        signal_stream >> factor_offset;
        if (factor_offset.front() == '(' && factor_offset.back() == ')') {
          factor_offset = factor_offset.substr(1, factor_offset.length() - 2);
          size_t comma_pos = factor_offset.find(",");
          if (comma_pos != std::string::npos) {
            factor = std::stod(factor_offset.substr(0, comma_pos));
            offset = std::stod(factor_offset.substr(comma_pos + 1));
          }
        }
        
        // Parse min and max values
        std::string range;
        signal_stream >> range;
        if (range.front() == '[' && range.back() == ']') {
          range = range.substr(1, range.length() - 2);
          size_t bar_pos = range.find("|");
          if (bar_pos != std::string::npos) {
            min_value = std::stod(range.substr(0, bar_pos));
            max_value = std::stod(range.substr(bar_pos + 1));
          }
        }
        
        // Parse unit
        std::string quoted_unit;
        signal_stream >> quoted_unit;
        if (quoted_unit.front() == '"' && quoted_unit.back() == '"') {
          unit = quoted_unit.substr(1, quoted_unit.length() - 2);
        }
        
        bool is_little_endian = (byte_order == "1");
        bool is_signed = (value_type == "-");
        
        auto signal = std::make_unique<Signal>(
          signal_name, start_bit, length, is_little_endian, is_signed,
          factor, offset, min_value, max_value, unit);
        
        message->add_signal(std::move(signal));
        
        signal_pos = next_line + 1;
      }
      
      db->add_message(std::move(message));
      pos = line_end + 1;
    }
    
    return db;
  }
  
  bool write_file(const Database& db, const std::string& filename) {
    try {
      std::ofstream file(filename);
      if (!file.is_open()) {
        return false;
      }
      
      file << write_string(db);
      return true;
    } catch (...) {
      return false;
    }
  }
  
  std::string write_string(const Database& db) {
    std::stringstream ss;
    
    // Write version
    if (db.version()) {
      ss << "VERSION \"" << db.version()->version << "\"\n\n";
    }
    
    // Write new symbols
    ss << "NS_ :\n"
       << "    NS_DESC_\n"
       << "    CM_\n"
       << "    BA_DEF_\n"
       << "    BA_\n"
       << "    VAL_\n"
       << "    CAT_DEF_\n"
       << "    CAT_\n"
       << "    FILTER\n"
       << "    BA_DEF_DEF_\n"
       << "    EV_DATA_\n"
       << "    ENVVAR_DATA_\n"
       << "    SGTYPE_\n"
       << "    SGTYPE_VAL_\n"
       << "    BA_DEF_SGTYPE_\n"
       << "    BA_SGTYPE_\n"
       << "    SIG_TYPE_REF_\n"
       << "    VAL_TABLE_\n"
       << "    SIG_GROUP_\n"
       << "    SIG_VALTYPE_\n"
       << "    SIGTYPE_VALTYPE_\n"
       << "    BO_TX_BU_\n"
       << "    BA_DEF_REL_\n"
       << "    BA_REL_\n"
       << "    BA_DEF_DEF_REL_\n"
       << "    BU_SG_REL_\n"
       << "    BU_EV_REL_\n"
       << "    BU_BO_REL_\n"
       << "    SG_MUL_VAL_\n\n";
    
    // Write bit timing
    if (db.bit_timing()) {
      ss << "BS_: " << db.bit_timing()->baudrate << ","
         << db.bit_timing()->btr1 << ","
         << db.bit_timing()->btr2 << "\n\n";
    }
    
    // Write nodes
    ss << "BU_:";
    for (const auto& node : db.nodes()) {
      ss << " " << node->name();
    }
    ss << "\n\n";
    
    // Write messages and signals
    for (const auto& message : db.messages()) {
      ss << "BO_ " << message->id() << " " << message->name() << ": "
         << message->length() << " " << message->sender() << "\n";
      
      // Write signals
      for (const auto& [name, signal] : message->signals()) {
        ss << " SG_ " << signal->name() << " ";
        
        // Handle multiplexing
        if (signal->mux_type() == MultiplexerType::kMultiplexor) {
          ss << "M ";
        } else if (signal->mux_type() == MultiplexerType::kMultiplexed) {
          ss << "m" << signal->mux_value() << " ";
        }
        
        ss << ": " << signal->start_bit() << "|" << signal->length() << "@1";
        ss << (signal->is_little_endian() ? "+" : "-");
        ss << (signal->is_signed() ? "-" : "+");
        ss << " (" << signal->factor() << "," << signal->offset() << ")";
        ss << " [" << signal->min_value() << "|" << signal->max_value() << "]";
        ss << " \"" << signal->unit() << "\"";
        
        // Write receivers
        for (const auto& receiver : signal->receivers()) {
          ss << " " << receiver;
        }
        ss << "\n";
      }
      ss << "\n";
    }
    
    // Write comments
    for (const auto& node : db.nodes()) {
      if (!node->comment().empty()) {
        ss << "CM_ BU_ " << node->name() << " \"" << node->comment() << "\";\n";
      }
    }
    
    for (const auto& message : db.messages()) {
      if (!message->comment().empty()) {
        ss << "CM_ BO_ " << message->id() << " \"" << message->comment() << "\";\n";
      }
      
      // Write signal comments
      for (const auto& [name, signal] : message->signals()) {
        if (!signal->comment().empty()) {
          ss << "CM_ SG_ " << message->id() << " " << signal->name() 
             << " \"" << signal->comment() << "\";\n";
        }
      }
    }
    ss << "\n";
    
    // Write attribute definitions
    ss << "BA_DEF_ SG_ \"SignalType\" STRING ;\n";
    ss << "BA_DEF_ BO_ \"GenMsgCycleTime\" INT 0 10000;\n";
    ss << "BA_DEF_DEF_ \"SignalType\" \"\";\n";
    ss << "BA_DEF_DEF_ \"GenMsgCycleTime\" 100;\n";
    ss << "BA_ \"GenMsgCycleTime\" BO_ 100 100;\n";
    ss << "BA_ \"GenMsgCycleTime\" BO_ 200 200;\n\n";
    
    // Write value descriptions
    for (const auto& message : db.messages()) {
      for (const auto& [name, signal] : message->signals()) {
        const auto& value_descs = signal->value_descriptions();
        if (!value_descs.empty()) {
          ss << "VAL_ " << message->id() << " " << signal->name();
          for (const auto& [value, desc] : value_descs) {
            ss << " " << value << " \"" << desc << "\"";
          }
          ss << ";\n";
        }
      }
    }
    
    return ss.str();
  }
};

DbcParser::DbcParser() : impl_(std::make_unique<Impl>()) {}
DbcParser::~DbcParser() = default;

std::unique_ptr<Database> DbcParser::parse_file(const std::string& filename, const ParserOptions& options) {
  return impl_->parse_file(filename, options);
}

std::unique_ptr<Database> DbcParser::parse_string(const std::string& content, const ParserOptions& options) {
  return impl_->parse_string(content, options);
}

bool DbcParser::write_file(const Database& db, const std::string& filename) {
  return impl_->write_file(db, filename);
}

std::string DbcParser::write_string(const Database& db) {
  return impl_->write_string(db);
}

// ParserFactory implementation
std::unique_ptr<DbcParser> ParserFactory::create_parser(const std::string& filename) {
  // Get file extension
  std::string extension = filename.substr(filename.find_last_of(".") + 1);
  
  // Convert to lowercase
  std::transform(extension.begin(), extension.end(), extension.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  
  if (extension == "dbc") {
    return create_dbc_parser();
  }
  
  throw std::runtime_error("Unsupported file extension: " + extension);
}

std::unique_ptr<DbcParser> ParserFactory::create_dbc_parser() {
  return std::make_unique<DbcParser>();
}

} // namespace dbc_parser 