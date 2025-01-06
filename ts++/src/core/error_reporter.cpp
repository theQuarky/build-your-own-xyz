#include "error_reporter.h"
#include <iostream>

void ErrorReporter::reportError(const std::string &fileName, unsigned int line,
                                unsigned int column,
                                const std::string &message) {
  Diagnostic diag;
  diag.fileName = fileName;
  diag.line = line;
  diag.column = column;
  diag.message = message;
  errors_.push_back(diag);
}

const std::vector<Diagnostic> &ErrorReporter::getErrors() const {
  return errors_;
}

void ErrorReporter::printAllErrors() const {
  for (const auto &err : errors_) {
    std::cerr << err.fileName << "(" << err.line << "," << err.column << "): "
              << "error: " << err.message << std::endl;
  }
}

void ErrorReporter::clear() { errors_.clear(); }
