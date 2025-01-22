/*****************************************************************************
 * File: common_types.h
 * Description: Core type definitions and utilities used throughout the compiler
 *
 * Contains:
 * - Basic type aliases
 * - Source location tracking
 * - Value storage and manipulation
 * - Error handling and result types
 *****************************************************************************/

#pragma once
#include <fstream>
#include <sstream>
#include <string>

namespace core {

/*****************************************************************************
 * Basic Type Aliases
 *****************************************************************************/
using Int = int;
using Float = float;
using Bool = bool;
using String = std::string;

/*****************************************************************************
 * Source Location & Error Display
 *****************************************************************************/
struct SourceLocation {
  // ANSI color codes for error highlighting
  const String RED = "\033[1;31m";
  const String RESET = "\033[0m";

  String filename;
  Int line;
  Int column;
  String line_content;

  SourceLocation() : line(0), column(0) {}

  SourceLocation(const String &file, Int l, Int c)
      : filename(file), line(l), column(c) {
    loadLineContent();
  }

  // Comparison operator for location checks
  Bool operator==(const SourceLocation &other) const {
    return filename == other.filename && line == other.line &&
           column == other.column;
  }

  // Load the source line for error display
  void loadLineContent() {
    std::ifstream file(filename);
    if (!file.is_open()) {
      line_content = "";
      return;
    }

    String current_line;
    Int current_line_num = 1;

    while (std::getline(file, current_line)) {
      if (current_line_num == line) {
        line_content = current_line;
        break;
      }
      current_line_num++;
    }

    file.close();
  }

  // Format location for error display
  String toString() const {
    std::stringstream output;
    // File location
    output << filename << ":" << line << ":" << column << "\n";

    // Show line content with error pointer if available
    if (!line_content.empty()) {
      output << line_content << "\n";
      String pointer_line(column - 1, ' ');
      output << pointer_line << RED << "^" << RESET;
    }

    return output.str();
  }
};

/*****************************************************************************
 * Value Storage
 *****************************************************************************/
union LiteralValue {
  Int int_value;
  Float float_value;
  Bool bool_value;

  // Value constructors
  LiteralValue() : int_value(0) {}
  explicit LiteralValue(Int v) : int_value(v) {}
  explicit LiteralValue(Float v) : float_value(v) {}
  explicit LiteralValue(Bool v) : bool_value(v) {}
};

/*****************************************************************************
 * Error Handling
 *****************************************************************************/
struct Error {
  String message;
  SourceLocation location;

  Error(String msg, SourceLocation loc)
      : message(std::move(msg)), location(std::move(loc)) {}

  String toString() const { return location.toString() + ": " + message; }
};

/*****************************************************************************
 * Result Type
 *****************************************************************************/
template <typename T> class Result {
private:
  T value_;
  Error error_;
  Bool has_error_;

public:
  // Constructors for success and failure cases
  Result(T value) : value_(std::move(value)), has_error_(false) {}
  Result(Error error) : error_(std::move(error)), has_error_(true) {}

  // Status checks
  Bool hasError() const { return has_error_; }
  const T &getValue() const { return value_; }
  const Error &getError() const { return error_; }

  // Implicit bool conversion for quick error checks
  operator bool() const { return !has_error_; }
};

} // namespace core