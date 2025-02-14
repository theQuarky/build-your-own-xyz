#include "base_parse_visitor.h"

namespace visitors {

BaseParseVisitor::BaseParseVisitor(tokens::TokenStream &tokens,
                                   core::ErrorReporter &errorReporter)
    : tokens_(tokens), errorReporter_(errorReporter) {
  // Initialize parsers in correct order due to dependencies
  expressionVisitor_ =
      std::make_unique<ExpressionParseVisitor>(tokens, errorReporter);

  statementVisitor_ = std::make_unique<StatementParseVisitor>(
      tokens, errorReporter, *expressionVisitor_);

  declarationVisitor_ = std::make_unique<DeclarationParseVisitor>(
      tokens, errorReporter, *expressionVisitor_, *statementVisitor_);
}

bool BaseParseVisitor::visitParse() {
  try {
    bool hadError = false;

    while (!tokens_.isAtEnd()) {
      if (isDeclarationStart()) {
        auto declNode = parseDeclaration();
        if (!declNode) {
          hadError = true;
          synchronize();
          continue;
        }
        nodes_.push_back(std::move(declNode));
      } else {
        auto stmtNode = parseStatement();
        if (!stmtNode) {
          hadError = true;
          synchronize();
          continue;
        }
        nodes_.push_back(std::move(stmtNode));
      }
    }

    return !hadError;
  } catch (const std::exception &e) {
    error(std::string("Error during parsing: ") + e.what());
    return false;
  }
}

nodes::NodePtr BaseParseVisitor::parseDeclaration() {
  return declarationVisitor_->parseDeclaration();
}

nodes::NodePtr BaseParseVisitor::parseStatement() {
  return statementVisitor_->parseStatement();
}

bool BaseParseVisitor::isDeclarationStart() const {
    // Check for storage class modifiers first
    if (tokens_.check(tokens::TokenType::STACK) || 
        tokens_.check(tokens::TokenType::HEAP) || 
        tokens_.check(tokens::TokenType::STATIC) ||
        tokens_.check(tokens::TokenType::ATTRIBUTE) ||
        tokens_.check(tokens::TokenType::LET) ||
        tokens_.check(tokens::TokenType::CONST) ||
        tokens_.check(tokens::TokenType::FUNCTION) ||
        tokens_.check(tokens::TokenType::CLASS)) {
        return true;
    }
    return false;
}

void BaseParseVisitor::synchronize() {
  tokens_.advance();

  while (!tokens_.isAtEnd()) {
    // Synchronize at statement boundaries
    if (tokens_.previous().getType() == tokens::TokenType::SEMICOLON) {
      return;
    }

    // Or at the start of major declarations/statements
    switch (tokens_.peek().getType()) {
    case tokens::TokenType::CLASS:
    case tokens::TokenType::FUNCTION:
    case tokens::TokenType::LET:
    case tokens::TokenType::CONST:
    case tokens::TokenType::IF:
    case tokens::TokenType::WHILE:
    case tokens::TokenType::RETURN:
      return;
    default:
      tokens_.advance();
    }
  }
}

void BaseParseVisitor::error(const std::string &message) {
  errorReporter_.error(tokens_.peek().getLocation(), message);
}

} // namespace visitors