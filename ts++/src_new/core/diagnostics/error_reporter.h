#pragma once
#include "../common/common_types.h"
#include <vector>

namespace core {

/**
 * @struct Diagnostic
 * @brief Represents a single error or warning in the source code
 */
struct Diagnostic {
  enum class Severity { Error, Warning, Info };

  Severity severity;       // Error severity level
  SourceLocation location; // Where the error occurred
  String message;          // Error description
  String code;             // Optional error code (e.g., "E001")

  Diagnostic(Severity sev, SourceLocation loc, String msg, String c = "")
      : severity(sev), location(std::move(loc)), message(std::move(msg)),
        code(std::move(c)) {}
};

/**
 * @class ErrorReporter
 * @brief Collects and manages diagnostic messages during compilation
 */
class ErrorReporter {
public:
  /**
   * @brief Reports an error at a specific location
   */
  void error(const SourceLocation &location, const String &message,
             const String &code = "");

  /**
   * @brief Reports a warning at a specific location
   */
  void warning(const SourceLocation &location, const String &message,
               const String &code = "");

  /**
   * @brief Reports an informational message at a specific location
   */
  void info(const SourceLocation &location, const String &message,
            const String &code = "");

  /**
   * @brief Gets all collected diagnostics
   */
  const std::vector<Diagnostic> &getDiagnostics() const { return diagnostics_; }

  /**
   * @brief Checks if any errors were reported
   */
  bool hasErrors() const { return errorCount_ > 0; }

  /**
   * @brief Gets the number of reported errors
   */
  u32 errorCount() const { return errorCount_; }

  /**
   * @brief Clears all collected diagnostics
   */
  void clear();

private:
  std::vector<Diagnostic> diagnostics_;
  u32 errorCount_ = 0;

  void report(Diagnostic::Severity severity, const SourceLocation &location,
              const String &message, const String &code);
};

} // namespace core