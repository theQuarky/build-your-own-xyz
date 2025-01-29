#include "expression_parse_visitor.h"
#include <iostream>

namespace visitors {

ExpressionParserVisitor::ExpressionParserVisitor(tokens::TokenStream &tokens)
    : tokens_(tokens) {}

// Main entry point for expressions
nodes::ExpressionPtr ExpressionParserVisitor::parseExpression() {
  return parseAssignmentExpr();
}

// Assignment expressions: a = b, a += b, etc.
nodes::ExpressionPtr ExpressionParserVisitor::parseAssignmentExpr() {
  auto expr = parseConditionalExpr();
  if (!expr)
    return nullptr;

  if (match(tokens::TokenType::EQUALS) ||
      match(tokens::TokenType::PLUS_EQUALS) ||
      match(tokens::TokenType::MINUS_EQUALS) ||
      match(tokens::TokenType::STAR_EQUALS) ||
      match(tokens::TokenType::SLASH_EQUALS) ||
      match(tokens::TokenType::PERCENT_EQUALS) ||
      match(tokens::TokenType::AMPERSAND_EQUALS) ||
      match(tokens::TokenType::PIPE_EQUALS) ||
      match(tokens::TokenType::CARET_EQUALS)) {

    auto op = tokens_.previous();
    auto value = parseAssignmentExpr();
    if (!value)
      return nullptr;

    return std::make_shared<nodes::AssignmentExpressionNode>(
        op.getLocation(), op.getType(), expr, value);
  }

  return expr;
}

// Conditional (ternary) expressions: a ? b : c
nodes::ExpressionPtr ExpressionParserVisitor::parseConditionalExpr() {
  auto expr = parseLogicalOrExpr();
  if (!expr)
    return nullptr;

  if (match(tokens::TokenType::QUESTION)) {
    auto trueExpr = parseExpression();
    if (!trueExpr)
      return nullptr;

    if (!match(tokens::TokenType::COLON)) {
      return error("Expected ':' in conditional expression");
    }

    auto falseExpr = parseConditionalExpr();
    if (!falseExpr)
      return nullptr;

    return std::make_shared<nodes::ConditionalExpressionNode>(
        expr->getLocation(), expr, trueExpr, falseExpr);
  }

  return expr;
}

// Logical OR expressions: a || b
nodes::ExpressionPtr ExpressionParserVisitor::parseLogicalOrExpr() {
  auto expr = parseLogicalAndExpr();
  if (!expr)
    return nullptr;

  while (match(tokens::TokenType::PIPE_PIPE)) {
    auto op = tokens_.previous();
    auto right = parseLogicalAndExpr();
    if (!right)
      return nullptr;
    expr = std::make_shared<nodes::BinaryExpressionNode>(
        op.getLocation(), op.getType(), expr, right);
  }

  return expr;
}

// Logical AND expressions: a && b
nodes::ExpressionPtr ExpressionParserVisitor::parseLogicalAndExpr() {
  auto expr = parseBitwiseOrExpr();
  if (!expr)
    return nullptr;

  while (match(tokens::TokenType::AMPERSAND_AMPERSAND)) {
    auto op = tokens_.previous();
    auto right = parseBitwiseOrExpr();
    if (!right)
      return nullptr;
    expr = std::make_shared<nodes::BinaryExpressionNode>(
        op.getLocation(), op.getType(), expr, right);
  }

  return expr;
}

// Bitwise operations
nodes::ExpressionPtr ExpressionParserVisitor::parseBitwiseOrExpr() {
  auto expr = parseBitwiseXorExpr();
  if (!expr)
    return nullptr;

  while (match(tokens::TokenType::PIPE)) {
    auto op = tokens_.previous();
    auto right = parseBitwiseXorExpr();
    if (!right)
      return nullptr;
    expr = std::make_shared<nodes::BinaryExpressionNode>(
        op.getLocation(), op.getType(), expr, right);
  }

  return expr;
}

nodes::ExpressionPtr ExpressionParserVisitor::parseBitwiseXorExpr() {
  auto expr = parseBitwiseAndExpr();
  if (!expr)
    return nullptr;

  while (match(tokens::TokenType::CARET)) {
    auto op = tokens_.previous();
    auto right = parseBitwiseAndExpr();
    if (!right)
      return nullptr;
    expr = std::make_shared<nodes::BinaryExpressionNode>(
        op.getLocation(), op.getType(), expr, right);
  }

  return expr;
}

nodes::ExpressionPtr ExpressionParserVisitor::parseBitwiseAndExpr() {
  auto expr = parseEqualityExpr();
  if (!expr)
    return nullptr;

  while (match(tokens::TokenType::AMPERSAND)) {
    auto op = tokens_.previous();
    auto right = parseEqualityExpr();
    if (!right)
      return nullptr;
    expr = std::make_shared<nodes::BinaryExpressionNode>(
        op.getLocation(), op.getType(), expr, right);
  }

  return expr;
}
// Add these implementations to expression_parse_visitor.cpp

nodes::ExpressionPtr
ExpressionParserVisitor::error(const std::string &message) {
  // TODO: Use error reporter for proper error handling
  // For now just print to stderr and return nullptr
  std::cerr << "Parse error: " << message << "\n";
  return nullptr;
}

nodes::ExpressionPtr ExpressionParserVisitor::parseArrayLiteral() {
  auto location = tokens_.previous().getLocation();
  std::vector<nodes::ExpressionPtr> elements;

  // Handle empty array []
  if (match(tokens::TokenType::RIGHT_BRACKET)) {
    return std::make_shared<nodes::ArrayLiteralNode>(location,
                                                     std::move(elements));
  }

  // Parse elements
  do {
    auto element = parseExpression();
    if (!element)
      return nullptr;
    elements.push_back(std::move(element));
  } while (match(tokens::TokenType::COMMA));

  if (!match(tokens::TokenType::RIGHT_BRACKET)) {
    return error("Expected ']' after array elements");
  }

  return std::make_shared<nodes::ArrayLiteralNode>(location,
                                                   std::move(elements));
}

nodes::ExpressionPtr ExpressionParserVisitor::parseCompileTimeExpr() {
  auto location = tokens_.previous().getLocation();
  auto kind = tokens_.previous().getType();

  // Parse the operand (what the compile-time operation is working on)
  if (!match(tokens::TokenType::LEFT_PAREN)) {
    return error("Expected '(' after compile-time operator");
  }

  auto operand = parseExpression();
  if (!operand)
    return nullptr;

  if (!match(tokens::TokenType::RIGHT_PAREN)) {
    return error("Expected ')' after compile-time expression");
  }

  return std::make_shared<nodes::CompileTimeExpressionNode>(location, kind,
                                                            std::move(operand));
}

// Equality and comparison
nodes::ExpressionPtr ExpressionParserVisitor::parseEqualityExpr() {
  auto expr = parseRelationalExpr();
  if (!expr)
    return nullptr;

  while (match(tokens::TokenType::EQUALS_EQUALS) ||
         match(tokens::TokenType::EXCLAIM_EQUALS)) {
    auto op = tokens_.previous();
    auto right = parseRelationalExpr();
    if (!right)
      return nullptr;
    expr = std::make_shared<nodes::BinaryExpressionNode>(
        op.getLocation(), op.getType(), expr, right);
  }

  return expr;
}

nodes::ExpressionPtr ExpressionParserVisitor::parseRelationalExpr() {
  auto expr = parseShiftExpr();
  if (!expr)
    return nullptr;

  while (match(tokens::TokenType::LESS) || match(tokens::TokenType::GREATER) ||
         match(tokens::TokenType::LESS_EQUALS) ||
         match(tokens::TokenType::GREATER_EQUALS)) {
    auto op = tokens_.previous();
    auto right = parseShiftExpr();
    if (!right)
      return nullptr;
    expr = std::make_shared<nodes::BinaryExpressionNode>(
        op.getLocation(), op.getType(), expr, right);
  }

  return expr;
}

// Shift operations
nodes::ExpressionPtr ExpressionParserVisitor::parseShiftExpr() {
  auto expr = parseAdditiveExpr();
  if (!expr)
    return nullptr;

  while (match(tokens::TokenType::LEFT_SHIFT) ||
         match(tokens::TokenType::RIGHT_SHIFT)) {
    auto op = tokens_.previous();
    auto right = parseAdditiveExpr();
    if (!right)
      return nullptr;
    expr = std::make_shared<nodes::BinaryExpressionNode>(
        op.getLocation(), op.getType(), expr, right);
  }

  return expr;
}

// Arithmetic operations
nodes::ExpressionPtr ExpressionParserVisitor::parseAdditiveExpr() {
  auto expr = parseMultiplicativeExpr();
  if (!expr)
    return nullptr;

  while (match(tokens::TokenType::PLUS) || match(tokens::TokenType::MINUS)) {
    auto op = tokens_.previous();
    auto right = parseMultiplicativeExpr();
    if (!right)
      return nullptr;
    expr = std::make_shared<nodes::BinaryExpressionNode>(
        op.getLocation(), op.getType(), expr, right);
  }

  return expr;
}

nodes::ExpressionPtr ExpressionParserVisitor::parseMultiplicativeExpr() {
  auto expr = parseUnaryExpr();
  if (!expr)
    return nullptr;

  while (match(tokens::TokenType::STAR) || match(tokens::TokenType::SLASH) ||
         match(tokens::TokenType::PERCENT)) {
    auto op = tokens_.previous();
    auto right = parseUnaryExpr();
    if (!right)
      return nullptr;
    expr = std::make_shared<nodes::BinaryExpressionNode>(
        op.getLocation(), op.getType(), expr, right);
  }

  return expr;
}

// Unary operations
nodes::ExpressionPtr ExpressionParserVisitor::parseUnaryExpr() {
  // Prefix operators
  if (match(tokens::TokenType::PLUS_PLUS) ||
      match(tokens::TokenType::MINUS_MINUS) || match(tokens::TokenType::PLUS) ||
      match(tokens::TokenType::MINUS) || match(tokens::TokenType::EXCLAIM) ||
      match(tokens::TokenType::TILDE) ||
      match(tokens::TokenType::AT)) { // Address-of operator

    auto op = tokens_.previous();
    auto right = parseUnaryExpr();
    if (!right)
      return nullptr;
    return std::make_shared<nodes::UnaryExpressionNode>(
        op.getLocation(), op.getType(), right, true);
  }

  return parsePostfixExpr();
}

// Postfix operations
nodes::ExpressionPtr ExpressionParserVisitor::parsePostfixExpr() {
  auto expr = parsePrimaryExpr();
  if (!expr)
    return nullptr;

  while (true) {
    if (match(tokens::TokenType::PLUS_PLUS) ||
        match(tokens::TokenType::MINUS_MINUS)) {
      // Postfix increment/decrement
      auto op = tokens_.previous();
      expr = std::make_shared<nodes::UnaryExpressionNode>(
          op.getLocation(), op.getType(), expr, false);
    } else if (match(tokens::TokenType::LEFT_BRACKET)) {
      // Array indexing
      auto index = parseExpression();
      if (!index)
        return nullptr;

      if (!match(tokens::TokenType::RIGHT_BRACKET)) {
        return error("Expected ']' after array index");
      }

      expr = std::make_shared<nodes::IndexExpressionNode>(expr->getLocation(),
                                                          expr, index);
    } else if (match(tokens::TokenType::LEFT_PAREN)) {
      // Function call
      std::vector<nodes::ExpressionPtr> arguments;

      if (!match(tokens::TokenType::RIGHT_PAREN)) {
        do {
          auto arg = parseExpression();
          if (!arg)
            return nullptr;
          arguments.push_back(std::move(arg));
        } while (match(tokens::TokenType::COMMA));

        if (!match(tokens::TokenType::RIGHT_PAREN)) {
          return error("Expected ')' after function arguments");
        }
      }

      expr = std::make_shared<nodes::CallExpressionNode>(
          expr->getLocation(), expr, std::move(arguments));
    } else if (match(tokens::TokenType::DOT) || match(tokens::TokenType::AT)) {
      // Member access
      bool isPointer = tokens_.previous().getType() == tokens::TokenType::AT;

      if (!match(tokens::TokenType::IDENTIFIER)) {
        return error("Expected identifier after '.' or '@'");
      }

      auto member = tokens_.previous().getLexeme();
      expr = std::make_shared<nodes::MemberExpressionNode>(
          expr->getLocation(), expr, member, isPointer);
    } else {
      break;
    }
  }

  return expr;
}

// Primary expressions
nodes::ExpressionPtr ExpressionParserVisitor::parsePrimaryExpr() {
  if (match(tokens::TokenType::IDENTIFIER)) {
    auto token = tokens_.previous();
    return std::make_shared<nodes::IdentifierExpressionNode>(
        token.getLocation(), token.getLexeme());
  }

  if (match(tokens::TokenType::NUMBER) ||
      match(tokens::TokenType::STRING_LITERAL) ||
      match(tokens::TokenType::TRUE) || match(tokens::TokenType::FALSE) ||
      match(tokens::TokenType::CHAR_LITERAL)) {
    auto token = tokens_.previous();
    return std::make_shared<nodes::LiteralExpressionNode>(
        token.getLocation(), token.getType(), token.getLexeme());
  }

  if (match(tokens::TokenType::NULL_VALUE) ||
      match(tokens::TokenType::UNDEFINED)) {
    auto token = tokens_.previous();
    return std::make_shared<nodes::LiteralExpressionNode>(
        token.getLocation(), token.getType(), token.getLexeme());
  }

  if (match(tokens::TokenType::THIS)) {
    auto token = tokens_.previous();
    return std::make_shared<nodes::ThisExpressionNode>(token.getLocation());
  }

  if (match(tokens::TokenType::LEFT_PAREN)) {
    auto expr = parseExpression();
    if (!expr)
      return nullptr;

    if (!match(tokens::TokenType::RIGHT_PAREN)) {
      return error("Expected ')' after expression");
    }
    return expr;
  }

  if (match(tokens::TokenType::LEFT_BRACKET)) {
    return parseArrayLiteral();
  }

  if (match(tokens::TokenType::NEW)) {
    return parseNewExpression();
  }

  // Compile-time expressions
  if (match(tokens::TokenType::CONST_EXPR) ||
      match(tokens::TokenType::SIZEOF) || match(tokens::TokenType::ALIGNOF) ||
      match(tokens::TokenType::TYPEOF)) {
    return parseCompileTimeExpr();
  }

  return error("Expected expression");
}

// Add this method to the ExpressionParserVisitor class implementation

nodes::ExpressionPtr ExpressionParserVisitor::parseNewExpression() {
  // Save location of 'new' keyword for error reporting
  auto location = tokens_.previous().getLocation();

  // Parse the type name - must be an identifier or qualified name
  std::string typeName;

  if (!match(tokens::TokenType::IDENTIFIER)) {
    return error("Expected type name after 'new'");
  }

  typeName = tokens_.previous().getLexeme();

  // Handle qualified names (e.g., namespace.Type)
  while (match(tokens::TokenType::DOT)) {
    if (!match(tokens::TokenType::IDENTIFIER)) {
      return error("Expected identifier after '.'");
    }
    typeName += "." + tokens_.previous().getLexeme();
  }

  // Parse constructor arguments if any
  std::vector<nodes::ExpressionPtr> arguments;

  if (match(tokens::TokenType::LEFT_PAREN)) {
    // Handle constructor arguments
    if (!match(tokens::TokenType::RIGHT_PAREN)) {
      do {
        auto arg = parseExpression();
        if (!arg)
          return nullptr;
        arguments.push_back(std::move(arg));
      } while (match(tokens::TokenType::COMMA));

      if (!match(tokens::TokenType::RIGHT_PAREN)) {
        return error("Expected ')' after constructor arguments");
      }
    }
  }

  return std::make_shared<nodes::NewExpressionNode>(
      location, std::move(typeName), std::move(arguments));
}

// Helper methods
bool ExpressionParserVisitor::match(tokens::TokenType type) {
  if (check(type)) {
    tokens_.advance();
    return true;
  }
  return false;
}

bool ExpressionParserVisitor::check(tokens::TokenType type) const {
  if (tokens_.isAtEnd())
    return false;
  return tokens_.peek().getType() == type;
}

} // namespace visitors