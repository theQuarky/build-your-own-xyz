#include "base_parse_visitor.h"

namespace visitors {

BaseParseVisitor::BaseParseVisitor(tokens::TokenStream &tokens,
                                   core::ErrorReporter &errorReporter)
    : tokens_(tokens), errorReporter_(errorReporter),
      expressionVisitor_(tokens),
      declarationVisitor_(tokens, errorReporter, expressionVisitor_) {}

bool BaseParseVisitor::visitParse() {
  try {
    while (!tokens_.isAtEnd()) {
      if (tokens_.check(tokens::TokenType::LET) ||
          tokens_.check(tokens::TokenType::CONST) ||
          tokens_.check(tokens::TokenType::FUNCTION) ||
          tokens_.check(tokens::TokenType::CLASS)) {

        auto declNode = parseDeclaration();
        if (!declNode)
          return false;
        nodes_.push_back(std::move(declNode)); // Store the node
      } else {
        auto stmtNode = parseStatement();
        if (!stmtNode)
          return false;
        nodes_.push_back(std::move(stmtNode)); // Store the node
      }
    }
    return true;
  } catch (const std::exception &e) {
    errorReporter_.error(tokens_.peek().getLocation(),
                         std::string("Error during parsing: ") + e.what());
    return false;
  }
}

nodes::NodePtr BaseParseVisitor::parseDeclaration() {
  return declarationVisitor_.parseDeclaration();
}

nodes::NodePtr BaseParseVisitor::parseStatement() {
  // Try to parse an expression statement
  auto expr = expressionVisitor_.parseExpression();
  if (!expr) {
    return nullptr;
  }

  // Make sure it's terminated by a semicolon
  if (!tokens_.match(tokens::TokenType::SEMICOLON)) {
    errorReporter_.error(tokens_.peek().getLocation(),
                         "Expected ';' after expression");
    return nullptr;
  }

  return expr;
}

} // namespace visitors