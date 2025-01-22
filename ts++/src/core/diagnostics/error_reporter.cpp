/*****************************************************************************
 * File: error_reporter.cpp
 * Description: Implementation of error reporting and diagnostic handling.
 *****************************************************************************/

#include "error_reporter.h"
#include <iostream>

namespace core {

/*****************************************************************************
 * Color Constants for Terminal Output
 *****************************************************************************/
namespace {
const String RED = "\033[1;31m";    // Error
const String YELLOW = "\033[1;33m"; // Warning
const String BLUE = "\033[1;34m";   // Info
const String RESET = "\033[0m";     // Reset color
} // namespace

/*****************************************************************************
 * Public Interface Implementation
 *****************************************************************************/
void ErrorReporter::error(const SourceLocation &location, const String &message,
                          const String &code) {
  report(Diagnostic::Severity::Error, location, message, code);
  errorCount_++;
}

void ErrorReporter::warning(const SourceLocation &location,
                            const String &message, const String &code) {
  report(Diagnostic::Severity::Warning, location, message, code);
}

void ErrorReporter::info(const SourceLocation &location, const String &message,
                         const String &code) {
  report(Diagnostic::Severity::Info, location, message, code);
}

void ErrorReporter::clear() {
  diagnostics_.clear();
  errorCount_ = 0;
}

/*****************************************************************************
 * Diagnostic Reporting Implementation
 *****************************************************************************/
void ErrorReporter::report(Diagnostic::Severity severity,
                           const SourceLocation &location,
                           const String &message, const String &code) {
  // Store diagnostic
  diagnostics_.emplace_back(severity, location, message, code);
  auto &diag = diagnostics_.back();

  // Select color based on severity
  const String &color = severity == Diagnostic::Severity::Error     ? RED
                        : severity == Diagnostic::Severity::Warning ? YELLOW
                                                                    : BLUE;

  // Get severity string
  const char *sevStr = severity == Diagnostic::Severity::Error     ? "error"
                       : severity == Diagnostic::Severity::Warning ? "warning"
                                                                   : "info";

  // Print location and severity
  std::cerr << location.filename << ":" << location.line << ":"
            << location.column << ": " << color << sevStr << RESET;

  // Print error code if present
  if (!diag.code.empty()) {
    std::cerr << "[" << diag.code << "]";
  }

  // Print message
  std::cerr << ": " << diag.message << "\n";

  // Print source line and error pointer
  if (!location.line_content.empty()) {
    std::cerr << location.line_content << "\n";
    String pointer_line(location.column - 1, ' ');
    std::cerr << pointer_line << color << "^" << RESET << "\n";
  }
}

/*****************************************************************************
 * Error Display Implementation
 *****************************************************************************/
void ErrorReporter::printAllErrors() const {
  for (const auto &diag : diagnostics_) {
    // Select color based on severity
    const String &color = diag.severity == Diagnostic::Severity::Error ? RED
                          : diag.severity == Diagnostic::Severity::Warning
                              ? YELLOW
                              : BLUE;

    const char *sevStr = diag.severity == Diagnostic::Severity::Error ? "error"
                         : diag.severity == Diagnostic::Severity::Warning
                             ? "warning"
                             : "info";

    // Print diagnostic header
    std::cerr << diag.location.filename << ":" << diag.location.line << ":"
              << diag.location.column << ": " << color << sevStr << RESET;

    // Print error code if present
    if (!diag.code.empty()) {
      std::cerr << " [" << diag.code << "]";
    }

    // Print message and source line
    std::cerr << ": " << diag.message << "\n";
    if (!diag.location.line_content.empty()) {
      std::cerr << diag.location.line_content << "\n";
      String pointer_line(diag.location.column - 1, ' ');
      std::cerr << pointer_line << color << "^" << RESET << "\n";
    }
  }
}

} // namespace core