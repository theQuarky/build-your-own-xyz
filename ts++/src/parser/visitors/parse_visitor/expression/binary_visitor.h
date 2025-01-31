#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/nodes/expression_nodes.h"
#include "tokens/stream/token_stream.h"
#include <functional>

namespace visitors {

class BinaryExpressionVisitor {
public:
  using UnaryCallback = std::function<nodes::ExpressionPtr()>;

  BinaryExpressionVisitor(tokens::TokenStream &tokens,
                          core::ErrorReporter &errorReporter)
      : tokens_(tokens), errorReporter_(errorReporter) {}

  void setUnaryParser(UnaryCallback callback) {
    unaryParser_ = std::move(callback);
  }

  inline nodes::ExpressionPtr parseBinary(nodes::ExpressionPtr left,
                                          int minPrecedence) {
    while (true) {
      auto token = tokens_.peek();
      if (!isBinaryOperator(token.getType()) ||
          getOperatorPrecedence(token.getType()) < minPrecedence) {
        break;
      }

      tokens_.advance();

      auto right = unaryParser_();
      if (!right)
        return nullptr;

      left = std::make_shared<nodes::BinaryExpressionNode>(
          token.getLocation(), token.getType(), left, right);
    }
    return left;
  }

private:
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  UnaryCallback unaryParser_;

  inline int getOperatorPrecedence(tokens::TokenType type) const {
    switch (type) {
    // Member access (highest)
    case tokens::TokenType::DOT: // .
    case tokens::TokenType::AT:  // @
      return 14;

    // Multiplicative
    case tokens::TokenType::STAR:    // *
    case tokens::TokenType::SLASH:   // /
    case tokens::TokenType::PERCENT: // %
      return 13;

    // Additive
    case tokens::TokenType::PLUS:  // +
    case tokens::TokenType::MINUS: // -
      return 12;

    // Shift
    case tokens::TokenType::LEFT_SHIFT:  // <<
    case tokens::TokenType::RIGHT_SHIFT: // >>
      return 11;

    // Relational
    case tokens::TokenType::LESS:           // <
    case tokens::TokenType::GREATER:        // >
    case tokens::TokenType::LESS_EQUALS:    // <=
    case tokens::TokenType::GREATER_EQUALS: // >=
      return 10;

    // Equality
    case tokens::TokenType::EQUALS_EQUALS:  // ==
    case tokens::TokenType::EXCLAIM_EQUALS: // !=
      return 9;

    // Bitwise AND
    case tokens::TokenType::AMPERSAND: // &
      return 8;

    // Bitwise XOR
    case tokens::TokenType::CARET: // ^
      return 7;

    // Bitwise OR
    case tokens::TokenType::PIPE: // |
      return 6;

    // Logical AND
    case tokens::TokenType::AMPERSAND_AMPERSAND: // &&
      return 5;

    // Logical OR
    case tokens::TokenType::PIPE_PIPE: // ||
      return 4;

    // Assignment
    case tokens::TokenType::EQUALS:           // =
    case tokens::TokenType::PLUS_EQUALS:      // +=
    case tokens::TokenType::MINUS_EQUALS:     // -=
    case tokens::TokenType::STAR_EQUALS:      // *=
    case tokens::TokenType::SLASH_EQUALS:     // /=
    case tokens::TokenType::PERCENT_EQUALS:   // %=
    case tokens::TokenType::AMPERSAND_EQUALS: // &=
    case tokens::TokenType::PIPE_EQUALS:      // |=
    case tokens::TokenType::CARET_EQUALS:     // ^=
      return 3;

    default:
      return -1; // Not a binary operator
    }
  }

  inline bool isBinaryOperator(tokens::TokenType type) const {
    // An operator is binary if it has a valid precedence
    // and isn't strictly unary
    if (getOperatorPrecedence(type) == -1) {
      return false;
    }

    // Special handling for operators that could be both unary and binary
    if (type == tokens::TokenType::PLUS || type == tokens::TokenType::MINUS) {
      return true; // These can be both unary and binary
    }

    // For all other cases, if it has a precedence and isn't in the unary-only
    // list
    static const std::vector<tokens::TokenType> unaryOnlyOps = {
        tokens::TokenType::TILDE,      // ~
        tokens::TokenType::EXCLAIM,    // !
        tokens::TokenType::PLUS_PLUS,  // ++
        tokens::TokenType::MINUS_MINUS // --
    };

    for (const auto &op : unaryOnlyOps) {
      if (type == op)
        return false;
    }

    return true;
  }
};
} // namespace visitors