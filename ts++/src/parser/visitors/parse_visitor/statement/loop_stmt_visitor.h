#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/nodes/statement_nodes.h"
#include "parser/visitors/parse_visitor/expression/iexpression_visitor.h"
#include "parser/visitors/parse_visitor/statement/istatement_visitor.h"
#include "tokens/stream/token_stream.h"

namespace visitors {

class ExpressionParseVisitor;
class StatementParseVisitor;

class LoopStatementVisitor {
public:
  LoopStatementVisitor(tokens::TokenStream &tokens,
                       core::ErrorReporter &errorReporter,
                       IExpressionVisitor &exprVisitor,
                       IStatementVisitor &stmtVisitor)
      : tokens_(tokens), errorReporter_(errorReporter),
        exprVisitor_(exprVisitor), stmtVisitor_(stmtVisitor) {}

  nodes::StmtPtr parseWhileStatement() {
    auto location = tokens_.previous().getLocation();

    if (!consume(tokens::TokenType::LEFT_PAREN, "Expected '(' after 'while'")) {
      return nullptr;
    }

    auto condition = exprVisitor_.parseExpression();
    if (!condition)
      return nullptr;

    if (!consume(tokens::TokenType::RIGHT_PAREN,
                 "Expected ')' after condition")) {
      return nullptr;
    }

    auto body = stmtVisitor_.parseStatement();
    if (!body)
      return nullptr;

    return std::make_shared<nodes::WhileStmtNode>(condition, body, location);
  }

  nodes::StmtPtr parseDoWhileStatement() {
    auto location = tokens_.previous().getLocation();

    auto body = stmtVisitor_.parseStatement();
    if (!body)
      return nullptr;

    if (!consume(tokens::TokenType::WHILE, "Expected 'while' after do block")) {
      return nullptr;
    }

    if (!consume(tokens::TokenType::LEFT_PAREN, "Expected '(' after 'while'")) {
      return nullptr;
    }

    auto condition = exprVisitor_.parseExpression();
    if (!condition)
      return nullptr;

    if (!consume(tokens::TokenType::RIGHT_PAREN,
                 "Expected ')' after condition")) {
      return nullptr;
    }

    if (!consume(tokens::TokenType::SEMICOLON,
                 "Expected ';' after do-while statement")) {
      return nullptr;
    }

    return std::make_shared<nodes::DoWhileStmtNode>(body, condition, location);
  }

  nodes::StmtPtr parseForStatement() {
    auto location = tokens_.previous().getLocation();

    if (!consume(tokens::TokenType::LEFT_PAREN, "Expected '(' after 'for'")) {
      return nullptr;
    }

    // Check for for-of statement
    if (match(tokens::TokenType::LET) || match(tokens::TokenType::CONST)) {
      return parseForOfStatement(location);
    }

    // Regular for loop
    nodes::StmtPtr initializer;
    if (!match(tokens::TokenType::SEMICOLON)) {
      initializer = parseForInitializer();
      if (!initializer)
        return nullptr;
    }

    nodes::ExpressionPtr condition;
    if (!match(tokens::TokenType::SEMICOLON)) {
      condition = exprVisitor_.parseExpression();
      if (!condition)
        return nullptr;

      if (!consume(tokens::TokenType::SEMICOLON,
                   "Expected ';' after loop condition")) {
        return nullptr;
      }
    }

    nodes::ExpressionPtr increment;
    if (!check(tokens::TokenType::RIGHT_PAREN)) {
      increment = exprVisitor_.parseExpression();
      if (!increment)
        return nullptr;
    }

    if (!consume(tokens::TokenType::RIGHT_PAREN,
                 "Expected ')' after for clauses")) {
      return nullptr;
    }

    auto body = stmtVisitor_.parseStatement();
    if (!body)
      return nullptr;

    return std::make_shared<nodes::ForStmtNode>(initializer, condition,
                                                increment, body, location);
  }

private:
  nodes::StmtPtr parseForOfStatement(const core::SourceLocation &location) {
    bool isConst = tokens_.previous().getType() == tokens::TokenType::CONST;

    if (!match(tokens::TokenType::IDENTIFIER)) {
      error("Expected variable name in for-of loop");
      return nullptr;
    }
    auto identifier = tokens_.previous().getLexeme();

    if (!consume(tokens::TokenType::OF, "Expected 'of' after variable name")) {
      return nullptr;
    }

    auto iterable = exprVisitor_.parseExpression();
    if (!iterable)
      return nullptr;

    if (!consume(tokens::TokenType::RIGHT_PAREN,
                 "Expected ')' after for-of clause")) {
      return nullptr;
    }

    auto body = stmtVisitor_.parseStatement();
    if (!body)
      return nullptr;

    return std::make_shared<nodes::ForOfStmtNode>(isConst, identifier, iterable,
                                                  body, location);
  }

  nodes::StmtPtr parseForInitializer() {
    // Declarations starting with let/const
    if (check(tokens::TokenType::LET) || check(tokens::TokenType::CONST)) {
      // TODO: Parse variable declaration
      return nullptr;
    }

    // Expression statement
    auto expr = exprVisitor_.parseExpression();
    if (!expr)
      return nullptr;

    if (!consume(tokens::TokenType::SEMICOLON,
                 "Expected ';' after loop initializer")) {
      return nullptr;
    }

    return std::make_shared<nodes::ExpressionStmtNode>(expr,
                                                       expr->getLocation());
  }

  inline bool match(tokens::TokenType type) {
    if (check(type)) {
      tokens_.advance();
      return true;
    }
    return false;
  }

  inline bool check(tokens::TokenType type) const {
    return !tokens_.isAtEnd() && tokens_.peek().getType() == type;
  }

  inline bool consume(tokens::TokenType type, const std::string &message) {
    if (check(type)) {
      tokens_.advance();
      return true;
    }
    error(message);
    return false;
  }

  inline void error(const std::string &message) {
    errorReporter_.error(tokens_.peek().getLocation(), message);
  }

  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  IExpressionVisitor &exprVisitor_;
  IStatementVisitor &stmtVisitor_;
};

} // namespace visitors