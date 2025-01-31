#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/nodes/expression_nodes.h"
#include "tokens/stream/token_stream.h"
#include <functional>

namespace visitors {

class UnaryExpressionVisitor {
public:
  using PrimaryCallback = std::function<nodes::ExpressionPtr()>;

  UnaryExpressionVisitor(tokens::TokenStream &tokens,
                         core::ErrorReporter &errorReporter)
      : tokens_(tokens), errorReporter_(errorReporter) {}

  void setPrimaryCallback(PrimaryCallback callback) {
    parsePrimaryCallback = std::move(callback);
  }
  inline nodes::ExpressionPtr parseUnary() {
    if (isUnaryOperator(tokens_.peek().getType())) {
      auto op = tokens_.peek();
      tokens_.advance();

      auto operand = parseUnary();
      if (!operand)
        return nullptr;

      return std::make_shared<nodes::UnaryExpressionNode>(op.getLocation(),
                                                          op.getType(), operand,
                                                          true // isPrefix
      );
    }

    return parsePrimary(); // Will be replaced with call to main visitor
  }

private:
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;

  inline bool isUnaryOperator(tokens::TokenType type) const {
    switch (type) {
    case tokens::TokenType::MINUS:
    case tokens::TokenType::EXCLAIM:
    case tokens::TokenType::TILDE:
    case tokens::TokenType::PLUS_PLUS:
    case tokens::TokenType::MINUS_MINUS:
      return true;
    default:
      return false;
    }
  }

  // Will be replaced with call to main visitor
  PrimaryCallback parsePrimaryCallback;
  nodes::ExpressionPtr parsePrimary() { return parsePrimaryCallback(); }
};
} // namespace visitors