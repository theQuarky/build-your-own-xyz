#include "statement_parse_visitor.h"

namespace visitors {

StatementParseVisitor::StatementParseVisitor(
    tokens::TokenStream &tokens, core::ErrorReporter &errorReporter,
    ExpressionParseVisitor &exprVisitor)
    : tokens_(tokens), errorReporter_(errorReporter), exprVisitor_(exprVisitor),
      branchVisitor_(tokens, errorReporter),
      loopVisitor_(tokens, errorReporter), tryVisitor_(tokens, errorReporter),
      flowVisitor_(tokens, errorReporter) {

  // Setup branch visitor callbacks
  branchVisitor_.setCallbacks(
      [this]() { return exprVisitor_.parseExpression(); },
      [this]() { return parseStatement(); });

  // Setup loop visitor callbacks
  loopVisitor_.setCallbacks([this]() { return exprVisitor_.parseExpression(); },
                            [this]() { return parseStatement(); });

  // Setup try-catch visitor callbacks
  tryVisitor_.setCallbacks(
      [this]() { return parseStatement(); },
      nullptr // TODO: Add type parsing callback once implemented
  );

  // Setup flow control visitor callback
  flowVisitor_.setExpressionCallback(
      [this]() { return exprVisitor_.parseExpression(); });
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
    error(std::string("Unexpected error while parsing statement: ") + e.what());
    synchronize();
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

bool StatementParseVisitor::match(tokens::TokenType type) {
  if (check(type)) {
    tokens_.advance();
    return true;
  }
  return false;
}

bool StatementParseVisitor::check(tokens::TokenType type) const {
  return !tokens_.isAtEnd() && tokens_.peek().getType() == type;
}

bool StatementParseVisitor::consume(tokens::TokenType type,
                                    const std::string &message) {
  if (check(type)) {
    tokens_.advance();
    return true;
  }
  error(message);
  return false;
}

void StatementParseVisitor::error(const std::string &message) {
  errorReporter_.error(tokens_.peek().getLocation(), message);
}

void StatementParseVisitor::synchronize() {
  tokens_.advance();

  while (!tokens_.isAtEnd()) {
    if (tokens_.previous().getType() == tokens::TokenType::SEMICOLON) {
      return;
    }

    switch (tokens_.peek().getType()) {
    case tokens::TokenType::CLASS:
    case tokens::TokenType::FUNCTION:
    case tokens::TokenType::LET:
    case tokens::TokenType::CONST:
    case tokens::TokenType::FOR:
    case tokens::TokenType::IF:
    case tokens::TokenType::WHILE:
    case tokens::TokenType::RETURN:
      return;
    default:
      break;
    }

    tokens_.advance();
  }
}

} // namespace visitors