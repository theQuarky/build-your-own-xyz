#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/visitors/parse_visitor/expression/iexpression_visitor.h"
#include "tokens/stream/token_stream.h"
#include "tokens/token_type.h"
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace visitors {

class ExpressionParseVisitor;

class PrimaryExpressionVisitor {
public:
  PrimaryExpressionVisitor(tokens::TokenStream &tokens,
                           core::ErrorReporter &errorReporter,
                           IExpressionVisitor &parent)
      : tokens_(tokens), errorReporter_(errorReporter), parentVisitor_(parent) {
  }

  // Parses a primary expression (array literals, "this", identifiers, literals,
  // or parenthesized expressions)
  nodes::ExpressionPtr parsePrimary() {
    // Handle array literals: [1, 2, 3]
    if (match(tokens::TokenType::LEFT_BRACKET)) {
      return parseArrayLiteral();
    }

    // Handle the "this" keyword
    if (match(tokens::TokenType::THIS)) {
      auto token = tokens_.previous();
      auto expr =
          std::make_shared<nodes::ThisExpressionNode>(token.getLocation());
      return parsePostfixOperations(expr);
    }

    // Handle identifiers
    if (match(tokens::TokenType::IDENTIFIER)) {
      auto token = tokens_.previous();
      auto expr = std::make_shared<nodes::IdentifierExpressionNode>(
          token.getLocation(), token.getLexeme());
      return parsePostfixOperations(expr);
    }

    // Handle literals (numbers, strings, booleans)
    if (match(tokens::TokenType::NUMBER) ||
        match(tokens::TokenType::STRING_LITERAL) ||
        match(tokens::TokenType::TRUE) || match(tokens::TokenType::FALSE)) {
      auto token = tokens_.previous();
      auto expr = std::make_shared<nodes::LiteralExpressionNode>(
          token.getLocation(), token.getType(), token.getLexeme());
      return parsePostfixOperations(expr);
    }

    // Handle parenthesized expressions: ( expression )
    if (match(tokens::TokenType::LEFT_PAREN)) {
      auto expr = parentVisitor_.parseExpression();
      if (!expr)
        return nullptr;
      if (!consume(tokens::TokenType::RIGHT_PAREN,
                   "Expected ')' after expression")) {
        return nullptr;
      }
      return parsePostfixOperations(expr);
    }

    error("Expected expression");
    return nullptr;
  }

  // Parses postfix operations such as member access, array indexing, and
  // function calls.
  nodes::ExpressionPtr parsePostfixOperations(nodes::ExpressionPtr expr) {
    while (true) {
      // Handle member access: e.g. obj.property (for "this._width")
      if (match(tokens::TokenType::DOT)) {
        if (tokens_.peek().getType() != tokens::TokenType::IDENTIFIER) {
          error("Expected property name after '.'");
          return nullptr;
        }
        // Advance and get the member token
        auto memberToken = tokens_.advance();
        std::string memberName = memberToken.getLexeme();

        // Create a MemberExpressionNode using the member token's location,
        // the current expression as the object, the member name, and false for
        // dot notation.
        expr = std::make_shared<nodes::MemberExpressionNode>(
            memberToken.getLocation(), // source location for the member
            expr,                      // object (e.g. "this")
            memberName,                // member name (e.g. "_width")
            false // isPointer flag (false for dot notation)
        );
      }
      // Handle array indexing: array[expression]
      else if (match(tokens::TokenType::LEFT_BRACKET)) {
        auto index = parentVisitor_.parseExpression();
        if (!index) {
          return nullptr;
        }
        if (!consume(tokens::TokenType::RIGHT_BRACKET,
                     "Expected ']' after array index")) {
          return nullptr;
        }
        expr = std::make_shared<nodes::IndexExpressionNode>(
            expr->getLocation(),
            expr, // the array expression
            index // the index expression
        );
      }
      // Handle function calls: func(arg1, arg2, ...)
      else if (match(tokens::TokenType::LEFT_PAREN)) {
        std::vector<nodes::ExpressionPtr> arguments;
        if (!check(tokens::TokenType::RIGHT_PAREN)) {
          do {
            auto arg = parentVisitor_.parseExpression();
            if (!arg) {
              return nullptr;
            }
            arguments.push_back(std::move(arg));
          } while (match(tokens::TokenType::COMMA));
        }
        if (!consume(tokens::TokenType::RIGHT_PAREN,
                     "Expected ')' after function arguments")) {
          return nullptr;
        }
        expr = std::make_shared<nodes::CallExpressionNode>(
            expr->getLocation(),
            expr,                // the callee expression
            std::move(arguments) // function arguments
        );
      } else {
        break; // No more postfix operations found.
      }
    }
    return expr;
  }

private:
  // Parses an array literal, e.g. [elem1, elem2, ...]
  nodes::ExpressionPtr parseArrayLiteral() {
    auto location = tokens_.previous().getLocation();
    std::vector<nodes::ExpressionPtr> elements;

    // Handle empty array literal.
    if (check(tokens::TokenType::RIGHT_BRACKET)) {
      tokens_.advance();
      return std::make_shared<nodes::ArrayLiteralNode>(location,
                                                       std::move(elements));
    }

    // Parse the array elements separated by commas.
    do {
      auto element = parentVisitor_.parseExpression();
      if (!element)
        return nullptr;
      elements.push_back(std::move(element));
    } while (match(tokens::TokenType::COMMA));

    if (!consume(tokens::TokenType::RIGHT_BRACKET,
                 "Expected ']' after array elements")) {
      return nullptr;
    }

    return std::make_shared<nodes::ArrayLiteralNode>(location,
                                                     std::move(elements));
  }

  // Utility: If the next token matches 'type', advance and return true.
  bool match(tokens::TokenType type) {
    if (check(type)) {
      tokens_.advance();
      return true;
    }
    return false;
  }

  // Utility: Check if the next token is of type 'type'.
  bool check(tokens::TokenType type) const {
    return !tokens_.isAtEnd() && tokens_.peek().getType() == type;
  }

  // Utility: Consume a token of type 'type' or report an error with 'message'.
  bool consume(tokens::TokenType type, const std::string &message) {
    if (check(type)) {
      tokens_.advance();
      return true;
    }
    error(message);
    return false;
  }

  // Report an error at the current token's location.
  void error(const std::string &message) {
    errorReporter_.error(tokens_.peek().getLocation(), message);
  }

  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  IExpressionVisitor &parentVisitor_;
};

} // namespace visitors
