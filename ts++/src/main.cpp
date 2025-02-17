#include "core/diagnostics/error_reporter.h"
#include "core/utils/file_utils.h"
#include "core/utils/log_utils.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "repl/repl.h"
#include <iostream>

int main(int argc, char *argv[]) {
  try {
    core::ErrorReporter errorReporter;

    if (argc == 1) {
      repl::Repl repl(errorReporter);
      repl.start();
      return 0;
    }

    const std::string filePath = argv[1];

    if (core::utils::FileUtils::getExtension(filePath) != "tspp") {
      std::cerr << "Error: File must have .tspp extension\n";
      return 1;
    }

    if (!core::utils::FileUtils::exists(filePath)) {
      std::cerr << "Error: File does not exist: " << filePath << "\n";
      return 1;
    }

    auto sourceCode = core::utils::FileUtils::readFile(filePath);
    if (!sourceCode) {
      std::cerr << "Error: Could not read file: " << filePath << "\n";
      return 1;
    }

    // Lexical analysis
    lexer::Lexer lexer(*sourceCode, filePath);
    auto tokens = lexer.tokenize();

    if (tokens.empty()) {
      std::cerr << "Fatal errors occurred during lexical analysis.\n";
      return 1;
    }

    // Print tokens if you want to see the lexer output
    printTokenStream(tokens);

    // Parsing
    parser::Parser parser(std::move(tokens), errorReporter);
    if (!parser.parse()) {
      // std::cerr << "Fatal errors occurred during parsing.\n";
      // errorReporter.printAllErrors();
      return 1;
    }

    // Get AST for next phase
    const auto &ast = parser.getAST();
    // TODO: Next phases (type checking, optimization, code generation)
    printAST(ast);
    return 0;

  } catch (const std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << "\n";
    return 1;
  }
}