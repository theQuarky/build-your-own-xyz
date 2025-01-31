#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/nodes/type_nodes.h"
#include "tokens/stream/token_stream.h"
#include <functional>

namespace visitors {

class CastExpressionVisitor {
public:
  using ExprCallback = std::function<nodes::ExpressionPtr()>;
  using TypeCallback = std::function<nodes::TypePtr()>;

  CastExpressionVisitor(tokens::TokenStream &tokens,
                        core::ErrorReporter &errorReporter)
      : tokens_(tokens), errorReporter_(errorReporter) {}

  void setCallbacks(ExprCallback exprCb, TypeCallback typeCb) {
    parseExpr_ = std::move(exprCb);
    parseType_ = std::move(typeCb);
  }

  nodes::ExpressionPtr parseCast() {
    auto location = tokens_.peek().getLocation();

    // Skip 'cast'
    tokens_.advance();

    if (!match(tokens::TokenType::LESS)) {
      error("Expected '<' after 'cast'");
      return nullptr;
    }

    auto targetType = parseType_();
    if (!targetType)
      return nullptr;

    if (!match(tokens::TokenType::GREATER)) {
      error("Expected '>' after type in cast expression");
      return nullptr;
    }

    auto expression = parseExpr_();
    if (!expression)
      return nullptr;

    return std::make_shared<nodes::CastExpressionNode>(
        location, targetType->toString(), expression);
  }

private:
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  ExprCallback parseExpr_;
  TypeCallback parseType_;

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

  void error(const std::string &message) {
    errorReporter_.error(tokens_.peek().getLocation(), message);
  }
};
} // namespace visitors