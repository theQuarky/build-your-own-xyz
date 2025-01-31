#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/nodes/declaration_nodes.h"
#include "tokens/stream/token_stream.h"
#include <functional>

namespace visitors {

class VariableDeclarationVisitor {
public:
  using ExprCallback = std::function<nodes::ExpressionPtr()>;
  using TypeCallback = std::function<nodes::TypePtr()>;

  VariableDeclarationVisitor(tokens::TokenStream &tokens,
                             core::ErrorReporter &errorReporter)
      : tokens_(tokens), errorReporter_(errorReporter) {}

  void setCallbacks(ExprCallback exprCb, TypeCallback typeCb) {
    parseExpr_ = std::move(exprCb);
    parseType_ = std::move(typeCb);
  }

  nodes::DeclPtr parseVarDecl(bool isConst, tokens::TokenType storageClass) {
    auto location = tokens_.peek().getLocation();

    // Get identifier
    if (!match(tokens::TokenType::IDENTIFIER)) {
      error("Expected variable name");
      return nullptr;
    }
    auto name = tokens_.previous().getLexeme();

    // Handle type annotation
    nodes::TypePtr type;
    if (match(tokens::TokenType::COLON)) {
      type = parseType_();
      if (!type)
        return nullptr;
    }

    // Handle initializer
    nodes::ExpressionPtr initializer;
    if (match(tokens::TokenType::EQUALS)) {
      initializer = parseExpr_();
      if (!initializer)
        return nullptr;
    } else if (isConst) {
      error("Const declarations must have an initializer");
      return nullptr;
    }

    // Expect semicolon
    if (!consume(tokens::TokenType::SEMICOLON,
                 "Expected ';' after variable declaration")) {
      return nullptr;
    }

    return std::make_shared<nodes::VarDeclNode>(
        name, type, initializer, storageClass, isConst, location);
  }

private:
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  ExprCallback parseExpr_;
  TypeCallback parseType_;

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