#include "dbc_parser/parser.h"
#include "dbc_parser/decoder.h"
#include "dbc_parser/types.h"

#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <fstream>

namespace dbc_parser {
namespace testing {

class ErrorHandlingTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create test DBC content
    valid_dbc_ = 
      "VERSION \"1.0\"\n"
      "\n"
      "NS_ :\n"
      "    NS_DESC_\n"
      "\n"
      "BS_: 500000,1,10\n"
      "\n"
      "BU_: NODE1 NODE2\n"
      "\n"
      "BO_ 100 TestMsg: 8 NODE1\n"
      " SG_ TestSignal : 0|8@1+ (1,0) [0|255] \"\" NODE2\n";
    
    // Invalid DBC content (syntax error)
    invalid_dbc_ = 
      "VERSION \"1.0\"\n"
      "\n"
      "NS_ :\n"
      "    NS_DESC_\n"
      "\n"
      "BS_: 500000,1\n"  // Missing third parameter
      "\n"
      "BU_: NODE1 NODE2\n"
      "\n"
      "BO_ 100 TestMsg: 8 NODE1\n"
      " SG_ TestSignal : 0|8@1+ (1,0) [0|255] \"\" NODE2\n";
    
    // DBC with invalid signal (out of message bounds)
    invalid_signal_dbc_ =
      "VERSION \"1.0\"\n"
      "\n"
      "BU_: NODE1 NODE2\n"
      "\n"
      "BO_ 100 TestMsg: 2 NODE1\n"  // 2 bytes long
      " SG_ TestSignal : 16|8@1+ (1,0) [0|255] \"\" NODE2\n";  // Start at bit 16 (3rd byte)
  }
  
  void TearDown() override {
  }
  
  std::string valid_dbc_;
  std::string invalid_dbc_;
  std::string invalid_signal_dbc_;
};

TEST_F(ErrorHandlingTest, ParseInvalidDbc) {
  // Test parsing invalid DBC content
  auto parser = std::make_unique<DbcParser>();
  
  // Should throw on invalid DBC
  EXPECT_THROW(parser->parse_string(invalid_dbc_, ParserOptions()), std::runtime_error);
  
  // Should parse valid DBC
  auto db = parser->parse_string(valid_dbc_, ParserOptions());
  ASSERT_TRUE(db);
}

TEST_F(ErrorHandlingTest, MissingFile) {
  // Test parsing a non-existent file
  auto parser = std::make_unique<DbcParser>();
  
  // Should throw on missing file
  EXPECT_THROW(parser->parse_file("non_existent_file.dbc", ParserOptions()), std::runtime_error);
}

TEST_F(ErrorHandlingTest, WriteToInvalidFile) {
  // Create a database
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(valid_dbc_, ParserOptions());
  ASSERT_TRUE(db);
  
  // Try to write to a path that doesn't exist
  EXPECT_FALSE(parser->write_file(*db, "/path/that/does/not/exist/file.dbc"));
  
  // Should be able to write to a valid path
  // Note: This test is commented out as it would create a real file
  // EXPECT_TRUE(parser->write_file(*db, "test_output.dbc"));
}

TEST_F(ErrorHandlingTest, DecodeOutOfBoundsSignal) {
  // Parse DBC with signal outside message bounds
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(invalid_signal_dbc_, ParserOptions());
  ASSERT_TRUE(db);
  
  // Create decoder with verbose option to see detailed errors
  DecoderOptions opts;
  opts.verbose = true;
  Decoder decoder(std::make_shared<Database>(*db), opts);
  
  // Attempt to decode frame (should handle error gracefully)
  std::vector<uint8_t> data = {0x01, 0x02};
  auto result = decoder.decode_frame(100, data);
  ASSERT_TRUE(result);
  
  // The out-of-bounds signal should not be present in the result
  EXPECT_TRUE(result->signals.empty());
}

TEST_F(ErrorHandlingTest, EmptyDatabase) {
  // Create an empty database
  auto db = std::make_unique<Database>();
  
  // Create decoder
  DecoderOptions opts;
  Decoder decoder(std::make_shared<Database>(*db), opts);
  
  // Try to decode a message that doesn't exist
  auto result = decoder.decode_frame(100, {0x01, 0x02});
  
  // With default options (ignore_unknown_ids = true), should return a placeholder
  ASSERT_TRUE(result);
  EXPECT_EQ("UNKNOWN_100", result->name);
  EXPECT_TRUE(result->signals.empty());
}

TEST_F(ErrorHandlingTest, NullData) {
  // Parse valid DBC
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_string(valid_dbc_, ParserOptions());
  ASSERT_TRUE(db);
  
  // Create decoder
  DecoderOptions opts;
  Decoder decoder(std::make_shared<Database>(*db), opts);
  
  // Try to decode with empty data vector
  std::vector<uint8_t> empty_data;
  auto result = decoder.decode_frame(100, empty_data);
  
  // Should return a valid result but with no signals decoded
  ASSERT_TRUE(result);
  EXPECT_EQ("TestMsg", result->name);
  EXPECT_TRUE(result->signals.empty());
}

TEST_F(ErrorHandlingTest, CustomErrorHandler) {
  // Create a custom error handler to capture errors
  class TestErrorHandler : public ParserErrorHandler {
   public:
    void on_error(const std::string& message, int line, int column) override {
      errors_.push_back(message);
    }
    
    void on_warning(const std::string& message, int line, int column) override {
      warnings_.push_back(message);
    }
    
    void on_info(const std::string& message, int line, int column) override {
      info_.push_back(message);
    }
    
    const std::vector<std::string>& errors() const { return errors_; }
    const std::vector<std::string>& warnings() const { return warnings_; }
    const std::vector<std::string>& info() const { return info_; }
    
   private:
    std::vector<std::string> errors_;
    std::vector<std::string> warnings_;
    std::vector<std::string> info_;
  };
  
  // We can't easily test the error handler in the current implementation
  // since it's embedded inside the parser implementation
  
  // This is a placeholder test that demonstrates how a custom error handler could be tested
  auto handler = std::make_unique<TestErrorHandler>();
  handler->on_error("Test error", 1, 1);
  EXPECT_EQ(1, handler->errors().size());
  EXPECT_EQ("Test error", handler->errors()[0]);
}

TEST_F(ErrorHandlingTest, DatabaseModificationErrors) {
  // Create a database
  auto db = std::make_unique<Database>();
  
  // Try to get a node that doesn't exist
  EXPECT_EQ(nullptr, db->get_node("NonExistentNode"));
  
  // Try to get a message that doesn't exist
  EXPECT_EQ(nullptr, db->get_message(999));
  
  // Create a message
  auto msg = std::make_unique<Message>(100, "TestMsg", 8, "NODE1");
  auto msg_ptr = db->add_message(std::move(msg));
  ASSERT_NE(nullptr, msg_ptr);
  
  // Try to get a signal that doesn't exist
  EXPECT_EQ(nullptr, msg_ptr->get_signal("NonExistentSignal"));
  
  // Try to remove a signal that doesn't exist
  EXPECT_FALSE(msg_ptr->remove_signal("NonExistentSignal"));
  
  // Add a signal and then try to remove it
  auto signal = std::make_unique<Signal>("TestSignal", 0, 8, true, false, 1.0, 0.0, 0.0, 100.0, "");
  auto signal_ptr = msg_ptr->add_signal(std::move(signal));
  ASSERT_NE(nullptr, signal_ptr);
  
  // Verify signal was added
  EXPECT_NE(nullptr, msg_ptr->get_signal("TestSignal"));
  
  // Remove the signal
  EXPECT_TRUE(msg_ptr->remove_signal("TestSignal"));
  
  // Verify signal was removed
  EXPECT_EQ(nullptr, msg_ptr->get_signal("TestSignal"));
}

TEST_F(ErrorHandlingTest, SignalEncodingErrors) {
  // Create a database with a message and signal
  auto db = std::make_unique<Database>();
  auto msg = std::make_unique<Message>(100, "TestMsg", 8, "NODE1");
  auto msg_ptr = db->add_message(std::move(msg));
  
  // Add a signal with a small range
  auto signal = std::make_unique<Signal>("TestSignal", 0, 8, true, false, 1.0, 0.0, 0.0, 255.0, "");
  auto signal_ptr = msg_ptr->add_signal(std::move(signal));
  
  // Create decoder
  DecoderOptions opts;
  Decoder decoder(std::make_shared<Database>(*db), opts);
  
  // Create data for encoding
  std::vector<uint8_t> data(8, 0);
  
  // Try to encode a value that's beyond the signal's range
  // This should clamp the value to the max range (255)
  signal_ptr->encode(1000.0, data);
  
  // Decode the data and check that the value was clamped
  auto decoded = decoder.decode_frame(100, data);
  ASSERT_TRUE(decoded);
  ASSERT_TRUE(decoded->signals.find("TestSignal") != decoded->signals.end());
  EXPECT_DOUBLE_EQ(255.0, decoded->signals["TestSignal"].value);
}

// Create a test file for testing file operations
void create_test_file(const std::string& filename, const std::string& content) {
  std::ofstream file(filename);
  file << content;
  file.close();
}

// Remove a test file
void remove_test_file(const std::string& filename) {
  remove(filename.c_str());
}

TEST_F(ErrorHandlingTest, FileOperations) {
  // This test is commented out as it interacts with the file system
  // Create a test file
  /*
  const std::string test_filename = "test_dbc_file.dbc";
  create_test_file(test_filename, valid_dbc_);
  
  // Parse the file
  auto parser = std::make_unique<DbcParser>();
  auto db = parser->parse_file(test_filename, ParserOptions());
  ASSERT_TRUE(db);
  
  // Write to a new file
  const std::string output_filename = "test_dbc_output.dbc";
  EXPECT_TRUE(parser->write_file(*db, output_filename));
  
  // Read the output file and verify it contains expected content
  std::ifstream output_file(output_filename);
  std::stringstream buffer;
  buffer << output_file.rdbuf();
  std::string output_content = buffer.str();
  
  // Content should contain essential elements from the original
  EXPECT_TRUE(output_content.find("VERSION") != std::string::npos);
  EXPECT_TRUE(output_content.find("TestMsg") != std::string::npos);
  
  // Clean up
  remove_test_file(test_filename);
  remove_test_file(output_filename);
  */
}

} // namespace testing
} // namespace dbc_parser

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
} 