#pragma once
#include "../core/error_reporter.h"
#include "../lexer/token.h"
#include "ast.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace parser {

class Parser {
public:
  Parser(std::vector<lexer::Token> tokens, ErrorReporter &errorReporter);
  std::vector<ast::StmtPtr> parse();

private:
  struct VariableInfo {
    bool isConst;
  };

  struct Scope {
    std::shared_ptr<Scope> parent;
    std::unordered_map<std::string, VariableInfo> variables;
  };

  std::vector<lexer::Token> tokens_;
  ErrorReporter &errorReporter_;
  size_t current_;
  std::shared_ptr<Scope> currentScope_;

  // Scope management
  void enterScope();
  void exitScope();
  bool isDeclared(const std::string &name) const;
  bool isConstant(const std::string &name) const;
  void declareVariable(const std::string &name, bool isConst);

  // Expression parsing methods
  ast::ExprPtr expression();
  ast::ExprPtr assignment();
  ast::ExprPtr logicalOr();
  ast::ExprPtr logicalAnd();
  ast::ExprPtr equality();
  ast::ExprPtr comparison();
  ast::ExprPtr term();
  ast::ExprPtr factor();
  ast::ExprPtr unary();
  ast::ExprPtr call();
  ast::ExprPtr primary();
  ast::ExprPtr bitOr();  // New
  ast::ExprPtr bitXor(); // New
  ast::ExprPtr bitAnd(); // New
  ast::ExprPtr shift();  // New
  // Statement parsing methods
  ast::StmtPtr declaration();
  ast::StmtPtr varDeclaration();
  ast::StmtPtr functionDeclaration();
  ast::StmtPtr statement();
  ast::StmtPtr expressionStatement();
  ast::StmtPtr ifStatement();
  ast::StmtPtr returnStatement();
  std::vector<ast::StmtPtr> block();

  // Type parsing
  ast::TypePtr type();

  // Utility methods
  bool match(lexer::TokenType type);
  bool check(lexer::TokenType type) const;
  lexer::Token advance();
  lexer::Token peek() const;
  lexer::Token previous() const;
  bool isAtEnd() const;
  lexer::Token consume(lexer::TokenType type, const std::string &message);
  void synchronize();
  void error(const std::string &message);
  bool isBlockEnd() const;
};

class ParserError : public std::runtime_error {
public:
  explicit ParserError(const std::string &message)
      : std::runtime_error(message) {}
};

} // namespace parser