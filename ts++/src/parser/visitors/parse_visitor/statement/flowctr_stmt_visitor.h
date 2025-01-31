#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/nodes/statement_nodes.h"
#include "tokens/stream/token_stream.h"
#include <functional>

namespace visitors {

class FlowControlVisitor {
public:
  using ExprCallback = std::function<nodes::ExpressionPtr()>;

  FlowControlVisitor(tokens::TokenStream &tokens,
                     core::ErrorReporter &errorReporter)
      : tokens_(tokens), errorReporter_(errorReporter) {}

  void setExpressionCallback(ExprCallback callback) {
    parseExpr_ = std::move(callback);
  }

  nodes::StmtPtr parseReturn() {
    auto location = tokens_.previous().getLocation();
    nodes::ExpressionPtr value;

    // Check for value after return
    if (!check(tokens::TokenType::SEMICOLON)) {
      value = parseExpr_();
      if (!value)
        return nullptr;
    }

    if (!consume(tokens::TokenType::SEMICOLON,
                 "Expected ';' after return statement")) {
      return nullptr;
    }

    return std::make_shared<nodes::ReturnStmtNode>(value, location);
  }

  nodes::StmtPtr parseBreak() {
    auto location = tokens_.previous().getLocation();
    std::string label;

    // Optional label
    if (match(tokens::TokenType::IDENTIFIER)) {
      label = tokens_.previous().getLexeme();
    }

    if (!consume(tokens::TokenType::SEMICOLON,
                 "Expected ';' after break statement")) {
      return nullptr;
    }

    return std::make_shared<nodes::BreakStmtNode>(std::move(label), location);
  }

  nodes::StmtPtr parseContinue() {
    auto location = tokens_.previous().getLocation();
    std::string label;

    // Optional label
    if (match(tokens::TokenType::IDENTIFIER)) {
      label = tokens_.previous().getLexeme();
    }

    if (!consume(tokens::TokenType::SEMICOLON,
                 "Expected ';' after continue statement")) {
      return nullptr;
    }

    return std::make_shared<nodes::ContinueStmtNode>(std::move(label),
                                                     location);
  }

  nodes::StmtPtr parseThrow() {
    auto location = tokens_.previous().getLocation();

    auto value = parseExpr_();
    if (!value)
      return nullptr;

    if (!consume(tokens::TokenType::SEMICOLON,
                 "Expected ';' after throw statement")) {
      return nullptr;
    }

    return std::make_shared<nodes::ThrowStmtNode>(value, location);
  }

private:
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  ExprCallback parseExpr_;

  bool match(tokens::TokenType type) {
    if (check(type)) {
      tokens_.advance();
      return true;
    }
    return false;
  }

  bool check(tokens::TokenType type) const {
    return !tokens_.isAtEnd() && tokens_.peek().getType() == type;
  }

  bool consume(tokens::TokenType type, const std::string &message) {
    if (check(type)) {
      tokens_.advance();
      return true;
    }
    error(message);
    return false;
  }

  void error(const std::string &message) {
    errorReporter_.error(tokens_.peek().getLocation(), message);
  }
};
} // namespace visitors