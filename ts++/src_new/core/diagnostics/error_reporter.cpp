#include "error_reporter.h"
#include <iostream>

namespace core {

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

void ErrorReporter::report(Diagnostic::Severity severity,
                           const SourceLocation &location,
                           const String &message, const String &code) {
  diagnostics_.emplace_back(severity, location, message, code);

  // Print diagnostic immediately for better debugging
  auto &diag = diagnostics_.back();
  const char *sevStr = severity == Diagnostic::Severity::Error     ? "error"
                       : severity == Diagnostic::Severity::Warning ? "warning"
                                                                   : "info";

  std::cerr << diag.location.filename << ":" << diag.location.line << ":"
            << diag.location.column << ": " << sevStr;

  if (!diag.code.empty()) {
    std::cerr << "[" << diag.code << "]";
  }

  std::cerr << ": " << diag.message << std::endl;
}

} // namespace core