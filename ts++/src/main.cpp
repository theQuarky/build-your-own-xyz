#include "core/error_reporter.h"
#include "core/utils.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

void processCode(const std::string &sourceCode, const std::string &fileName,
                 bool isRepl = false) {
  ErrorReporter errorReporter;

  // Lexical analysis
  lexer::Lexer lexer(sourceCode, fileName, errorReporter);
  auto tokens = lexer.tokenize();

  if (isRepl) {
    std::cout << "\nTokens:\n";
    printTokens(tokens);
  }

  // Only proceed with parsing if we have valid tokens
  if (!errorReporter.getErrors().empty()) {
    std::cout << "\nLexer Errors:\n";
    errorReporter.printAllErrors();
    return;
  }

  // Parsing
  try {
    parser::Parser parser(tokens, errorReporter);
    auto ast = parser.parse();

    if (isRepl) {
      std::cout << "\nAST Structure:\n";
      if (ast.empty()) {
        std::cout << "<empty AST>\n";
      } else {
        printAST(ast);
      }
    }

    // Print any parser errors
    if (!errorReporter.getErrors().empty()) {
      std::cout << "\nParser Errors:\n";
      errorReporter.printAllErrors();
    }
  } catch (const parser::ParserError &e) {
    std::cerr << "Parser error: " << e.what() << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Unexpected error: " << e.what() << std::endl;
  }

  if (isRepl) {
    std::cout << std::endl; // Extra newline for readability in REPL mode
  }
}

void startRepl() {
  std::cout << "TS++ REPL v1.0.0\n";
  std::cout << "Type 'exit' or press Ctrl+D to exit\n";
  std::cout << "Type your code and press Enter. For multiline input, end with "
               "a blank line.\n\n";

  int lineNumber = 1;

  while (true) {
    std::cout << "ts++ > ";
    std::string line;
    std::string accumulator;
    bool isMultiline = false;

    while (std::getline(std::cin, line)) {
      if (line == "exit") {
        std::cout << "Goodbye!\n";
        return;
      }

      if (line.empty() && isMultiline) {
        break;
      }

      if (!line.empty()) {
        accumulator += line + "\n";
        isMultiline = true;
      } else if (!isMultiline) {
        break;
      }
    }

    if (std::cin.eof()) {
      std::cout << "\nGoodbye!\n";
      break;
    }

    if (!accumulator.empty()) {
      processCode(accumulator, "repl_" + std::to_string(lineNumber) + ".tspp",
                  true);
      lineNumber++;
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc > 2) {
    std::cerr << "Usage: " << argv[0] << " [source_file.tspp]\n";
    return 1;
  }

  // No arguments - start REPL
  if (argc == 1) {
    startRepl();
    return 0;
  }

  // Process source file
  std::string sourcePath = argv[1];
  if (std::filesystem::path(sourcePath).extension() != ".tspp") {
    std::cerr << "Error: Source file must have .tspp extension\n";
    return 1;
  }

  try {
    std::ifstream file(sourcePath);
    if (!file.is_open()) {
      std::cerr << "Error: Could not open file: " << sourcePath << std::endl;
      return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string sourceCode = buffer.str();

    std::cout << "Processing file: " << sourcePath << "\n";
    std::cout << "Source Code:\n" << sourceCode << std::endl;

    processCode(sourceCode, sourcePath, false);

  } catch (const std::exception &e) {
    std::cerr << "Error processing file " << sourcePath << ": " << e.what()
              << std::endl;
    return 1;
  }

  return 0;
}