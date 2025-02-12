#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/visitors/parse_visitor/expression/iexpression_visitor.h"
#include "tokens/stream/token_stream.h"

namespace visitors {

class ExpressionParseVisitor;

class UnaryExpressionVisitor {
public:
  UnaryExpressionVisitor(tokens::TokenStream &tokens,
                         core::ErrorReporter &errorReporter,
                         IExpressionVisitor &parent)
      : tokens_(tokens), errorReporter_(errorReporter), parentVisitor_(parent) {
  }

  nodes::ExpressionPtr parseUnary() {
    if (isUnaryOperator(tokens_.peek().getType())) {
      auto op = tokens_.peek();
      tokens_.advance();

      // Recursive call to handle nested unary operators
      auto operand = parseUnary();
      if (!operand)
        return nullptr;

      return std::make_shared<nodes::UnaryExpressionNode>(op.getLocation(),
                                                          op.getType(), operand,
                                                          true // isPrefix
      );
    }

    // If not a unary operator, parse as primary expression
    return parentVisitor_.parsePrimary();
  }

private:
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  IExpressionVisitor &parentVisitor_;

  bool isUnaryOperator(tokens::TokenType type) const {
    switch (type) {
    case tokens::TokenType::MINUS:       // -
    case tokens::TokenType::EXCLAIM:     // !
    case tokens::TokenType::TILDE:       // ~
    case tokens::TokenType::PLUS_PLUS:   // ++
    case tokens::TokenType::MINUS_MINUS: // --
      return true;
    default:
      return false;
    }
  }
};
} // namespace visitors