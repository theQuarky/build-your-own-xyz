#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/nodes/expression_nodes.h"
#include "tokens/stream/token_stream.h"
#include <functional>

namespace visitors {

class PrimaryExpressionVisitor {
public:
  PrimaryExpressionVisitor(tokens::TokenStream &tokens,
                           core::ErrorReporter &errorReporter)
      : tokens_(tokens), errorReporter_(errorReporter) {}
  using ExpressionCallback = std::function<nodes::ExpressionPtr()>;

  inline nodes::ExpressionPtr parsePrimary() {
    if (match(tokens::TokenType::IDENTIFIER)) {
      auto token = tokens_.previous();
      return std::make_shared<nodes::IdentifierExpressionNode>(
          token.getLocation(), token.getLexeme());
    }

    if (match(tokens::TokenType::NUMBER) ||
        match(tokens::TokenType::STRING_LITERAL) ||
        match(tokens::TokenType::TRUE) || match(tokens::TokenType::FALSE)) {
      auto token = tokens_.previous();
      return std::make_shared<nodes::LiteralExpressionNode>(
          token.getLocation(), token.getType(), token.getLexeme());
    }

    error("Expected expression");
    return nullptr;
  }
  void setExpressionCallback(ExpressionCallback callback) {
    parseExpressionCallback = std::move(callback);
  }

private:
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;

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

  inline void error(const std::string &message) {
    errorReporter_.error(tokens_.peek().getLocation(), message);
  }

  ExpressionCallback parseExpressionCallback;
  nodes::ExpressionPtr parseExpression() { return parseExpressionCallback(); }
};
} // namespace visitors