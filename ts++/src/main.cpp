#include "core/error_reporter.h"
#include "core/utils.h"
#include "lexer/lexer.h"
#include <iomanip>
#include <iostream>
#include <string>

int main() {
  // Test cases with various language features
  std::vector<std::string> testCases = {// Variable declarations and assignments
                                        R"(
            let x: int = 42;
            const PI: float = 3.14159;
            let message: string = "Hello, World!";
            let flag: boolean = true;
        )",

                                        // Operators and expressions
                                        R"(
            let a: int = 10;
            let b: int = 20;
            let result: int = (a + b) * 2;
            result += 5;
            result++;
            let bitwise: int = a & b | c ^ d;
            let shifted: int = value >>> 2;
        )",

                                        // Control flow
                                        R"(
            if (x > 0) {
                return x;
            } else {
                return -x;
            }
        )",

                                        // Function declaration
                                        R"(
            function add(a: int, b: int): int {
                return a + b;
            }
        )",

                                        // Semicolon and newline cases
                                        R"(
            let x = 10  // No semicolon, but newline (valid)
            let y = 20 let z = 30  // Multiple statements on one line (invalid)
            let a = 40; let b = 50; // Multiple statements with semicolons (valid)
            let c = 60
            let d = 70  // These are fine because they're on separate lines
            
            if (x > 0) 
                doSomething()  // No semicolon needed before closing brace
            
            let obj = {
                key: value  // No semicolon needed in object literal
            }
            
            function test() {
                return 42  // No semicolon needed before closing brace
            }
        )"};

  // Process each test case
  for (size_t i = 0; i < testCases.size(); ++i) {
    // std::cout << "\nTest Case #" << (i + 1) << ":\n";
    // std::cout << "Source Code:\n" << testCases[i] << std::endl;

    // Create error reporter
    ErrorReporter errorReporter;

    // Create lexer and process the source
    lexer::Lexer lexer(testCases[i], "test_" + std::to_string(i + 1) + ".tspp",
                       errorReporter);
    auto tokens = lexer.tokenize();

    // Print tokens
    // std::cout << "\nTokens:\n";
    // printTokens(tokens);

    // Print any errors
    if (!errorReporter.getErrors().empty()) {
      std::cout << "\nErrors:\n";
      errorReporter.printAllErrors();
    }

    // std::cout << "\n" << std::setfill('=') << std::setw(80) << "=" <<
    // std::endl;
  }

  return 0;
}