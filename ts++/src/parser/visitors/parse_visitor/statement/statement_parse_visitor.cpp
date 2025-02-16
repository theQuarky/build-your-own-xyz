#include "statement_parse_visitor.h"
#include "parser/nodes/declaration_nodes.h"

namespace visitors {

nodes::StmtPtr StatementParseVisitor::parseExpressionStatement() {
  auto location = tokens_.peek().getLocation();

  auto expr = exprVisitor_.parseExpression();
  if (!expr)
    return nullptr;

  if (!consume(tokens::TokenType::SEMICOLON, "Expected ';' after expression")) {
    return nullptr;
  }

  return std::make_shared<nodes::ExpressionStmtNode>(expr, location);
}

nodes::StmtPtr StatementParseVisitor::parseAssemblyStatement() {
  auto location = tokens_.previous().getLocation();

  if (!consume(tokens::TokenType::LEFT_PAREN, "Expected '(' after '#asm'")) {
    return nullptr;
  }

  if (!match(tokens::TokenType::STRING_LITERAL)) {
    error("Expected string literal containing assembly code");
    return nullptr;
  }

  std::string asmCode = tokens_.previous().getLexeme();
  std::vector<std::string> constraints;

  while (match(tokens::TokenType::COMMA)) {
    if (!match(tokens::TokenType::STRING_LITERAL)) {
      error("Expected constraint string");
      return nullptr;
    }
    constraints.push_back(tokens_.previous().getLexeme());
  }

  if (!consume(tokens::TokenType::RIGHT_PAREN,
               "Expected ')' after assembly code")) {
    return nullptr;
  }

  if (!consume(tokens::TokenType::SEMICOLON,
               "Expected ';' after assembly statement")) {
    return nullptr;
  }

  return std::make_shared<nodes::AssemblyStmtNode>(
      asmCode, std::move(constraints), location);
}

} // namespace visitors