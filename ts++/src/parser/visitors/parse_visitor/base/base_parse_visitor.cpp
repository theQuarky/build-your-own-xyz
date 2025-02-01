// base_parse_visitor.cpp
#include "base_parse_visitor.h"
#include <iostream>
#include <iterator>

namespace visitors {

BaseParseVisitor::BaseParseVisitor(tokens::TokenStream &tokens,
                                   core::ErrorReporter &errorReporter)
    : tokens_(tokens), errorReporter_(errorReporter),
      expressionVisitor_(tokens, errorReporter),
      statementVisitor_(tokens, errorReporter, expressionVisitor_),
      declarationVisitor_(tokens, errorReporter, expressionVisitor_,
                          statementVisitor_) {}

bool BaseParseVisitor::visitParse() {
  try {
    bool hadError = false;

    while (!tokens_.isAtEnd()) {
      // Handle declarations
      if (tokens_.check(tokens::TokenType::ATTRIBUTE) ||
          tokens_.check(tokens::TokenType::LET) ||
          tokens_.check(tokens::TokenType::CONST) ||
          tokens_.check(tokens::TokenType::FUNCTION) ||
          tokens_.check(tokens::TokenType::CLASS)) {
        std::cout << tokens_.peek().getLexeme() << std::endl;
        auto declNode = parseDeclaration();
        if (!declNode) {
          hadError = true;
          synchronize();
          continue;
        }
        nodes_.push_back(std::move(declNode));
      }
      // Handle statements
      else {
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
    errorReporter_.error(tokens_.peek().getLocation(),
                         std::string("Error during parsing: ") + e.what());
    return false;
  }
}

nodes::NodePtr BaseParseVisitor::parseDeclaration() {
  return declarationVisitor_.parseDeclaration();
}

nodes::NodePtr BaseParseVisitor::parseStatement() {
  return statementVisitor_.parseStatement();
}

bool BaseParseVisitor::match(tokens::TokenType type) {
  if (check(type)) {
    tokens_.advance();
    return true;
  }
  return false;
}

bool BaseParseVisitor::check(tokens::TokenType type) const {
  return !tokens_.isAtEnd() && tokens_.peek().getType() == type;
}

void BaseParseVisitor::error(const std::string &message) {
  errorReporter_.error(tokens_.peek().getLocation(), message);
}

void BaseParseVisitor::synchronize() {
  tokens_.advance();

  while (!tokens_.isAtEnd()) {
    if (tokens_.previous().getType() == tokens::TokenType::SEMICOLON)
      return;

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

} // namespace visitors