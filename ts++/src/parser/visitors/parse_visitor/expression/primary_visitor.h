#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/visitors/parse_visitor/expression/iexpression_visitor.h"
#include "tokens/stream/token_stream.h"
#include "tokens/token_type.h"
#include <iostream>
#include <ostream>

namespace visitors {

class ExpressionParseVisitor;

class PrimaryExpressionVisitor {
public:
  PrimaryExpressionVisitor(tokens::TokenStream &tokens,
                           core::ErrorReporter &errorReporter,
                           IExpressionVisitor &parent)
      : tokens_(tokens), errorReporter_(errorReporter), parentVisitor_(parent) {
  }

  nodes::ExpressionPtr parsePrimary() {
    // Handle array literals [1, 2, 3]
    if (match(tokens::TokenType::LEFT_BRACKET)) {
      return parseArrayLiteral();
    }

    // Handle identifiers
    if (match(tokens::TokenType::IDENTIFIER)) {
      auto token = tokens_.previous();
      return std::make_shared<nodes::IdentifierExpressionNode>(
          token.getLocation(), token.getLexeme());
    }

    // Handle literals
    if (match(tokens::TokenType::NUMBER) ||
        match(tokens::TokenType::STRING_LITERAL) ||
        match(tokens::TokenType::TRUE) || match(tokens::TokenType::FALSE)) {
      auto token = tokens_.previous();
      return std::make_shared<nodes::LiteralExpressionNode>(
          token.getLocation(), token.getType(), token.getLexeme());
    }

    // Handle parenthesized expressions
    if (match(tokens::TokenType::LEFT_PAREN)) {
      auto expr = parentVisitor_.parseExpression();
      if (!expr)
        return nullptr;

      if (!consume(tokens::TokenType::RIGHT_PAREN,
                   "Expected ')' after expression")) {
        return nullptr;
      }

      return expr;
    }
    std::cout << "current token: " << tokens_.getCurrentToken().getLexeme()
              << std::endl;

    error("Expected expression");
    return nullptr;
  }

private:
  // Add utility method to parse array literals
  nodes::ExpressionPtr parseArrayLiteral() {
    auto location = tokens_.previous().getLocation();
    std::vector<nodes::ExpressionPtr> elements;

    // Handle empty array
    if (check(tokens::TokenType::RIGHT_BRACKET)) {
      tokens_.advance();
      return std::make_shared<nodes::ArrayLiteralNode>(location,
                                                       std::move(elements));
    }

    // Parse array elements
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

  bool isDeclKeyword(tokens::TokenType type) const {
    switch (type) {
    case tokens::TokenType::LET:
    case tokens::TokenType::CONST:
    case tokens::TokenType::FUNCTION:
    case tokens::TokenType::CLASS:
    case tokens::TokenType::INTERFACE:
    case tokens::TokenType::STACK:
    case tokens::TokenType::HEAP:
    case tokens::TokenType::STATIC:
    case tokens::TokenType::INLINE:
    case tokens::TokenType::VIRTUAL:
    case tokens::TokenType::UNSAFE:
    case tokens::TokenType::SIMD:
      return true;
    default:
      return false;
    }
  }

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

  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  IExpressionVisitor &parentVisitor_;
};

} // namespace visitors