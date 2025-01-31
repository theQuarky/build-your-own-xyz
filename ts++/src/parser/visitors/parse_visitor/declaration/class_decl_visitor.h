#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/nodes/declaration_nodes.h"
#include "tokens/stream/token_stream.h"
#include <functional>

namespace visitors {

class ClassDeclarationVisitor {
public:
  using DeclCallback = std::function<nodes::DeclPtr()>;
  using TypeCallback = std::function<nodes::TypePtr()>;

  ClassDeclarationVisitor(tokens::TokenStream &tokens,
                          core::ErrorReporter &errorReporter)
      : tokens_(tokens), errorReporter_(errorReporter) {}

  void setCallbacks(DeclCallback declCb, TypeCallback typeCb) {
    parseDecl_ = std::move(declCb);
    parseType_ = std::move(typeCb);
  }

  nodes::DeclPtr parseClassDecl() {
    auto location = tokens_.peek().getLocation();

    // Parse class attributes
    std::vector<nodes::AttributePtr> attributes;
    while (match(tokens::TokenType::ATTRIBUTE)) {
      auto attr = parseAttribute();
      if (attr)
        attributes.push_back(std::move(attr));
    }

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
      baseClass = parseType_();
      if (!baseClass)
        return nullptr;
    }

    if (match(tokens::TokenType::IMPLEMENTS)) {
      do {
        auto interface = parseType_();
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
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  DeclCallback parseDecl_;
  TypeCallback parseType_;

  nodes::AttributePtr parseAttribute() {
    auto location = tokens_.previous().getLocation();

    if (!match(tokens::TokenType::IDENTIFIER)) {
      error("Expected attribute name");
      return nullptr;
    }
    auto name = tokens_.previous().getLexeme();

    nodes::ExpressionPtr argument;
    if (match(tokens::TokenType::LEFT_PAREN)) {
      // Parse attribute argument
      if (!match(tokens::TokenType::RIGHT_PAREN)) {
        error("Expected ')' after attribute argument");
        return nullptr;
      }
    }

    return std::make_shared<nodes::AttributeNode>(name, argument, location);
  }

  nodes::DeclPtr parseMemberDecl() {
    // Parse access modifier if present
    tokens::TokenType access = tokens::TokenType::PUBLIC;
    if (match(tokens::TokenType::PUBLIC) || match(tokens::TokenType::PRIVATE) ||
        match(tokens::TokenType::PROTECTED)) {
      access = tokens_.previous().getType();
    }

    return parseDecl_();
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
};
} // namespace visitors