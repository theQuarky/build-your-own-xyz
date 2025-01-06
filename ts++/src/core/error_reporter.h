#pragma once

#include <string>
#include <vector>

// A structure to hold individual error info
struct Diagnostic {
  std::string fileName; // which source file?
  unsigned int line;
  unsigned int column;
  std::string message;
};

// A central class to manage error reporting
class ErrorReporter {
public:
  // Add an error with location and message
  void reportError(const std::string &fileName, unsigned int line,
                   unsigned int column, const std::string &message);

  // Return all errors
  const std::vector<Diagnostic> &getErrors() const;

  // Print all errors in a friendly format
  void printAllErrors() const;

  // Clear errors (optional, if you want to reuse)
  void clear();

private:
  std::vector<Diagnostic> errors_;
};
