#pragma once
#include "core/diagnostics/error_reporter.h"
#include "ideclaration_visitor.h"
#include "parser/nodes/declaration_nodes.h"
#include "tokens/stream/token_stream.h"

namespace visitors {

class DeclarationParseVisitor;

class ClassDeclarationVisitor {
public:
  ClassDeclarationVisitor(tokens::TokenStream &tokens,
                          core::ErrorReporter &errorReporter,
                          IDeclarationVisitor &declVisitor)
      : tokens_(tokens), errorReporter_(errorReporter),
        declVisitor_(declVisitor) {}

  nodes::DeclPtr parseClassDecl() {
    auto location = tokens_.peek().getLocation();

    // Parse class name
    if (!match(tokens::TokenType::IDENTIFIER)) {
      error("Expected class name");
      return nullptr;
    }
    auto name = tokens_.previous().getLexeme();

    // Parse inheritance
    nodes::TypePtr baseClass;
    std::vector<nodes::TypePtr> interfaces;

    if (match(tokens::TokenType::EXTENDS)) {
      baseClass = declVisitor_.parseType();
      if (!baseClass)
        return nullptr;
    }

    if (match(tokens::TokenType::IMPLEMENTS)) {
      do {
        auto interface = declVisitor_.parseType();
        if (!interface)
          return nullptr;
        interfaces.push_back(std::move(interface));
      } while (match(tokens::TokenType::COMMA));
    }

    // Parse class body
    if (!match(tokens::TokenType::LEFT_BRACE)) {
      error("Expected '{' before class body");
      return nullptr;
    }

    std::vector<nodes::DeclPtr> members;
    while (!check(tokens::TokenType::RIGHT_BRACE) && !tokens_.isAtEnd()) {
      auto member = parseMemberDecl();
      if (member) {
        members.push_back(std::move(member));
      }
    }

    if (!consume(tokens::TokenType::RIGHT_BRACE,
                 "Expected '}' after class body")) {
      return nullptr;
    }

    // TODO: Return actual ClassDeclNode once implemented
    return nullptr;
  }

private:
  inline bool match(tokens::TokenType type) {
    if (check(type)) {
      tokens_.advance();
      return true;
    }
    return false;
  }

  inline bool check(tokens::TokenType type) const {
    return !tokens_.isAtEnd() && tokens_.peek().getType() == type;
  }

  inline bool consume(tokens::TokenType type, const std::string &message) {
    if (check(type)) {
      tokens_.advance();
      return true;
    }
    error(message);
    return false;
  }

  inline void error(const std::string &message) {
    errorReporter_.error(tokens_.peek().getLocation(), message);
  }

  nodes::DeclPtr parseMemberDecl() {
    // TODO: Implement member declaration parsing
    return nullptr;
  }

  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  IDeclarationVisitor &declVisitor_;
};
} // namespace visitors