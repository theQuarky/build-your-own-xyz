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
  assert(&tokens != nullptr && "Token stream cannot be null");
  assert(&errorReporter != nullptr && "Error reporter cannot be null");
}

nodes::ExpressionPtr ExpressionParseVisitor::parseExpression() {
  try {
    // Handle cast expressions
    if (tokens_.peek().getLexeme() == "cast") {
      return castVisitor_.parseCast();
    }

    // Parse expression chain
    auto expr = unaryVisitor_.parseUnary();
    if (!expr)
      return nullptr;

    expr = binaryVisitor_.parseBinary(expr, 0);
    if (!expr)
      return nullptr;

    return callVisitor_.parseCallOrMember(expr);
  } catch (const std::exception &e) {
    errorReporter_.error(tokens_.peek().getLocation(),
                         std::string("Error parsing expression: ") + e.what());
    return nullptr;
  }
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