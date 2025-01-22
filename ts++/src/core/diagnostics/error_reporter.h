/*****************************************************************************
 * File: error_reporter.h
 * Description: Error and diagnostic reporting system for the compiler.
 *
 * Handles:
 * - Collection of compiler diagnostics (errors, warnings, info)
 * - Error reporting with source location tracking
 * - Pretty-printing of error messages
 *****************************************************************************/

#pragma once
#include "../common/common_types.h"
#include <vector>

namespace core {

/*****************************************************************************
 * Diagnostic Information
 *****************************************************************************/
struct Diagnostic {
  // Severity levels for different message types
  enum class Severity {
    Error,   // Fatal errors that prevent compilation
    Warning, // Potential issues that don't stop compilation
    Info     // Informational messages for the user
  };

  Severity severity;       // Message severity level
  SourceLocation location; // Source code location
  String message;          // Descriptive message
  String code;             // Optional diagnostic code (e.g., "E001")

  Diagnostic(Severity sev, SourceLocation loc, String msg, String c = "")
      : severity(sev), location(std::move(loc)), message(std::move(msg)),
        code(std::move(c)) {}
};

/*****************************************************************************
 * Error Reporter
 *****************************************************************************/
class ErrorReporter {
public:
  /*****************************************************************************
   * Error Reporting Interface
   *****************************************************************************/
  // Report different types of diagnostics
  void error(const SourceLocation &location, const String &message,
             const String &code = "");

  void warning(const SourceLocation &location, const String &message,
               const String &code = "");

  void info(const SourceLocation &location, const String &message,
            const String &code = "");

  /*****************************************************************************
   * Diagnostic Access
   *****************************************************************************/
  // Access collected diagnostics
  const std::vector<Diagnostic> &getDiagnostics() const { return diagnostics_; }

  // Error state queries
  bool hasErrors() const { return errorCount_ > 0; }
  int errorCount() const { return errorCount_; }

  // Diagnostic management
  void clear();
  void printAllErrors() const;

private:
  /*****************************************************************************
   * Internal State
   *****************************************************************************/
  std::vector<Diagnostic> diagnostics_; // All collected diagnostics
  int errorCount_ = 0;                  // Number of errors encountered

  /*****************************************************************************
   * Helper Methods
   *****************************************************************************/
  // Common reporting logic
  void report(Diagnostic::Severity severity, const SourceLocation &location,
              const String &message, const String &code);
};

} // namespace core