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
    return parse(buffer.str(), options);
  }
  
  std::unique_ptr<Database> parse(const std::string& content, const ParserOptions& options) {
    ParserContext context;
    DefaultParserErrorHandler error_handler;
    
    using iterator_type = std::string::const_iterator;
    DbcGrammar<iterator_type> grammar(context, error_handler);
    
    bool result = qi::phrase_parse(
        content.begin(), content.end(),
        grammar,
        qi::space
    );
    
    if (!result) {
      throw std::runtime_error("Failed to parse DBC content");
    }
    
    return context.finalize();
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
  return impl_->parse(content, options);
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