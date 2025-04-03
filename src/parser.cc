#include "parser.h"

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/file_input.hpp>
#include <tao/pegtl/memory_input.hpp>

#include <iostream>
#include <sstream>
#include <stack>

#include "ast/dbc_ast.h"
#include "grammar/dbc_grammar.h" // Include the main grammar file
#include "grammar/dbc_actions.h" // Include the grammar actions file

namespace pegtl = tao::pegtl;

namespace dbc_parser {

bool DbcParser::Parse(const std::string& file_path, std::unique_ptr<ast::DbcFile>* dbc_file) {
  if (!dbc_file) {
    last_error_ = "Output parameter dbc_file cannot be null.";
    return false;
  }
  dbc_file->reset(); // Ensure the output pointer is cleared initially

  try {
    pegtl::file_input<> in(file_path);
    ParsingState state;
    
    // First, analyze the grammar (optional, good for debugging)
    // if (pegtl::analyze<grammar::dbc_file>() != 0) {
    //   last_error_ = "Grammar analysis failed.";
    //   return false;
    // }
    
    // Parse the file using the grammar and actions
    pegtl::parse<grammar::dbc_file, grammar::action>(in, state);

    // If parsing is successful, transfer ownership of the created DbcFile
    *dbc_file = std::move(state.dbc_file);
    last_error_ = ""; // Clear any previous errors
    return true;

  } catch (const pegtl::parse_error& e) {
    const auto pos = e.positions().front();
    std::ostringstream oss;
    oss << "Parse error at " << pos << ": " << e.what();
    last_error_ = oss.str();
    dbc_file->reset(); // Ensure output is null on error
    return false;
  } catch (const std::exception& e) {
    last_error_ = "An unexpected error occurred during parsing: ";
    last_error_ += e.what();
    dbc_file->reset();
    return false;
  }
}

bool DbcParser::ParseString(const std::string& content, const std::string& source_name,
                          std::unique_ptr<ast::DbcFile>* dbc_file) {
  if (!dbc_file) {
    last_error_ = "Output parameter dbc_file cannot be null.";
    return false;
  }
  dbc_file->reset();

  try {
    pegtl::memory_input<> in(content, source_name);
    ParsingState state;

    // Parse the string content using the grammar and actions
    pegtl::parse<grammar::dbc_file, grammar::action>(in, state);

    // If parsing is successful, transfer ownership
    *dbc_file = std::move(state.dbc_file);
    last_error_ = "";
    return true;

  } catch (const pegtl::parse_error& e) {
    const auto pos = e.positions().front();
    std::ostringstream oss;
    oss << "Parse error at " << pos << " in '" << source_name << "': " << e.what();
    last_error_ = oss.str();
    dbc_file->reset();
    return false;
  } catch (const std::exception& e) {
    last_error_ = "An unexpected error occurred during string parsing: ";
    last_error_ += e.what();
    dbc_file->reset();
    return false;
  }
}

std::string DbcParser::GetLastError() const {
  return last_error_;
}

}  // namespace dbc_parser 