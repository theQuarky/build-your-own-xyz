#include "expression_parse_visitor.h"

namespace visitors {

ExpressionParseVisitor::ExpressionParseVisitor(
    tokens::TokenStream &tokens, core::ErrorReporter &errorReporter)
    : tokens_(tokens), errorReporter_(errorReporter),
      binaryVisitor_(tokens, errorReporter, *this),
      unaryVisitor_(tokens, errorReporter, *this),
      primaryVisitor_(tokens, errorReporter, *this),
      callVisitor_(tokens, errorReporter, *this),
      castVisitor_(tokens, errorReporter, *this) {
  // Validate constructor parameters
  // assert(&tokens != nullptr && "Token stream cannot be null");
  // assert(&errorReporter != nullptr && "Error reporter cannot be null");
}

nodes::ExpressionPtr ExpressionParseVisitor::parseExpression() {
  try {
    // Start with assignment parsing since it has lowest precedence
    auto expr = parseAssignment();
    if (!expr)
      return nullptr;

    return callVisitor_.parseCallOrMember(expr);
  } catch (const std::exception &e) {
    errorReporter_.error(tokens_.peek().getLocation(),
                         std::string("Error parsing expression: ") + e.what());
    return nullptr;
  }
}

nodes::ExpressionPtr ExpressionParseVisitor::parseAssignment() {
  // First parse left side (which could be an identifier or a more complex
  // expression)
  auto expr = parseAdditive();
  if (!expr)
    return nullptr;

  // Check if this is an assignment
  if (match(tokens::TokenType::EQUALS) ||
      match(tokens::TokenType::PLUS_EQUALS) ||
      match(tokens::TokenType::MINUS_EQUALS) ||
      match(tokens::TokenType::STAR_EQUALS) ||
      match(tokens::TokenType::SLASH_EQUALS)) {

    auto op = tokens_.previous().getType();

    // Parse right side of assignment
    auto value = parseAssignment(); // Assignments are right-associative
    if (!value)
      return nullptr;

    // Create assignment node
    expr = std::make_shared<nodes::AssignmentExpressionNode>(
        expr->getLocation(), op, expr, value);
  }

  return expr;
}

nodes::ExpressionPtr ExpressionParseVisitor::parseAdditive() {
  auto expr = parseMultiplicative();
  if (!expr)
    return nullptr;

  while (match(tokens::TokenType::PLUS) || match(tokens::TokenType::MINUS)) {
    auto op = tokens_.previous().getType();
    auto right = parseMultiplicative();
    if (!right)
      return nullptr;

    expr = std::make_shared<nodes::BinaryExpressionNode>(expr->getLocation(),
                                                         op, expr, right);
  }

  return expr;
}

nodes::ExpressionPtr ExpressionParseVisitor::parseMultiplicative() {
  auto expr = parseUnary();
  if (!expr)
    return nullptr;

  while (match(tokens::TokenType::STAR) || match(tokens::TokenType::SLASH)) {
    auto op = tokens_.previous().getType();
    auto right = parseUnary();
    if (!right)
      return nullptr;

    expr = std::make_shared<nodes::BinaryExpressionNode>(expr->getLocation(),
                                                         op, expr, right);
  }

  return expr;
}

nodes::ExpressionPtr ExpressionParseVisitor::parsePrimary() {
  return primaryVisitor_.parsePrimary();
}

nodes::ExpressionPtr ExpressionParseVisitor::parseUnary() {
  return unaryVisitor_.parseUnary();
}

nodes::TypePtr ExpressionParseVisitor::parseType() {
  if (!check(tokens::TokenType::IDENTIFIER)) {
    error("Expected type name");
    return nullptr;
  }

  auto location = tokens_.peek().getLocation();
  auto name = tokens_.peek().getLexeme();
  tokens_.advance();

  return std::make_shared<nodes::NamedTypeNode>(name, location);
}

bool ExpressionParseVisitor::match(tokens::TokenType type) {
  if (check(type)) {
    tokens_.advance();
    return true;
  }
  return false;
}

bool ExpressionParseVisitor::check(tokens::TokenType type) const {
  return !tokens_.isAtEnd() && tokens_.peek().getType() == type;
}

void ExpressionParseVisitor::error(const std::string &message) {
  errorReporter_.error(tokens_.peek().getLocation(), message);
}

} // namespace visitors