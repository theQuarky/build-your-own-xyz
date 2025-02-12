#include "statement_parse_visitor.h"
#include "parser/nodes/declaration_nodes.h"

namespace visitors {

StatementParseVisitor::StatementParseVisitor(
    tokens::TokenStream &tokens, core::ErrorReporter &errorReporter,
    IExpressionVisitor &exprVisitor)
    : tokens_(tokens), errorReporter_(errorReporter), exprVisitor_(exprVisitor),
      branchVisitor_(tokens, errorReporter, exprVisitor, *this),
      loopVisitor_(tokens, errorReporter, exprVisitor, *this),
      flowVisitor_(tokens, errorReporter, exprVisitor),
      tryVisitor_(tokens, errorReporter, *this) {
  assert(&tokens != nullptr && "Token stream cannot be null");
  assert(&errorReporter != nullptr && "Error reporter cannot be null");
}

nodes::StmtPtr StatementParseVisitor::parseStatement() {
  try {
    if (match(tokens::TokenType::LEFT_BRACE)) {
      return parseBlock();
    }
    if (match(tokens::TokenType::IF)) {
      return branchVisitor_.parseIfStatement();
    }
    if (match(tokens::TokenType::SWITCH)) {
      return branchVisitor_.parseSwitchStatement();
    }
    if (match(tokens::TokenType::WHILE)) {
      return loopVisitor_.parseWhileStatement();
    }
    if (match(tokens::TokenType::DO)) {
      return loopVisitor_.parseDoWhileStatement();
    }
    if (match(tokens::TokenType::FOR)) {
      return loopVisitor_.parseForStatement();
    }
    if (match(tokens::TokenType::TRY)) {
      return tryVisitor_.parseTryStatement();
    }
    if (match(tokens::TokenType::RETURN)) {
      return flowVisitor_.parseReturn();
    }
    if (match(tokens::TokenType::BREAK)) {
      return flowVisitor_.parseBreak();
    }
    if (match(tokens::TokenType::CONTINUE)) {
      return flowVisitor_.parseContinue();
    }
    if (match(tokens::TokenType::THROW)) {
      return flowVisitor_.parseThrow();
    }
    if (match(tokens::TokenType::ATTRIBUTE) &&
        tokens_.peek().getLexeme() == "asm") {
      return parseAssemblyStatement();
    }

    return parseExpressionStatement();
  } catch (const std::exception &e) {
    error(std::string("Error parsing statement: ") + e.what());
    return nullptr;
  }
}

nodes::BlockPtr StatementParseVisitor::parseBlock() {
  auto location = tokens_.previous().getLocation();
  std::vector<nodes::StmtPtr> statements;

  while (!check(tokens::TokenType::RIGHT_BRACE) && !tokens_.isAtEnd()) {
    if (auto stmt = parseStatement()) {
      statements.push_back(std::move(stmt));
    }
  }

  if (!consume(tokens::TokenType::RIGHT_BRACE, "Expected '}' after block")) {
    return nullptr;
  }

  return std::make_shared<nodes::BlockNode>(std::move(statements), location);
}

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