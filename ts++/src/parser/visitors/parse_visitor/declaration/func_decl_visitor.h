#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/nodes/declaration_nodes.h"
#include "tokens/stream/token_stream.h"
#include <functional>

namespace visitors {

class FunctionDeclarationVisitor {
public:
  using BlockCallback = std::function<nodes::BlockPtr()>;
  using TypeCallback = std::function<nodes::TypePtr()>;

  FunctionDeclarationVisitor(tokens::TokenStream &tokens,
                             core::ErrorReporter &errorReporter)
      : tokens_(tokens), errorReporter_(errorReporter) {}

  void setCallbacks(BlockCallback blockCb, TypeCallback typeCb) {
    parseBlock_ = std::move(blockCb);
    parseType_ = std::move(typeCb);
  }

  nodes::DeclPtr parseFuncDecl() {
    auto location = tokens_.peek().getLocation();

    // Parse function name
    if (!match(tokens::TokenType::IDENTIFIER)) {
      error("Expected function name");
      return nullptr;
    }
    auto name = tokens_.previous().getLexeme();

    // Parse parameter list
    if (!match(tokens::TokenType::LEFT_PAREN)) {
      error("Expected '(' after function name");
      return nullptr;
    }

    auto parameters = parseParameterList();

    // Parse return type
    nodes::TypePtr returnType;
    if (match(tokens::TokenType::COLON)) {
      returnType = parseType_();
      if (!returnType)
        return nullptr;
    }

    // Parse function body
    nodes::BlockPtr body;
    if (match(tokens::TokenType::LEFT_BRACE)) {
      body = parseBlock_();
      if (!body)
        return nullptr;
    } else {
      error("Expected '{' before function body");
      return nullptr;
    }

    return std::make_shared<nodes::FunctionDeclNode>(name, parameters,
                                                     returnType, body,
                                                     false, // isAsync
                                                     location);
  }

private:
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  BlockCallback parseBlock_;
  TypeCallback parseType_;

  std::vector<nodes::ParamPtr> parseParameterList() {
    std::vector<nodes::ParamPtr> parameters;

    if (!check(tokens::TokenType::RIGHT_PAREN)) {
      do {
        auto param = parseParameter();
        if (!param)
          return parameters;
        parameters.push_back(std::move(param));
      } while (match(tokens::TokenType::COMMA));
    }

    if (!consume(tokens::TokenType::RIGHT_PAREN,
                 "Expected ')' after parameters")) {
      return parameters;
    }

    return parameters;
  }

  nodes::ParamPtr parseParameter() {
    auto location = tokens_.peek().getLocation();

    bool isConst = match(tokens::TokenType::CONST);
    bool isRef = match(tokens::TokenType::REF);

    if (!match(tokens::TokenType::IDENTIFIER)) {
      error("Expected parameter name");
      return nullptr;
    }
    auto name = tokens_.previous().getLexeme();

    if (!consume(tokens::TokenType::COLON,
                 "Expected ':' after parameter name")) {
      return nullptr;
    }

    auto type = parseType_();
    if (!type)
      return nullptr;

    nodes::ExpressionPtr defaultValue;
    if (match(tokens::TokenType::EQUALS)) {
      // Parse default value
      defaultValue = parseDefaultValue();
      if (!defaultValue)
        return nullptr;
    }

    return std::make_shared<nodes::ParameterNode>(name, type, defaultValue,
                                                  isRef, isConst, location);
  }

  nodes::ExpressionPtr parseDefaultValue() {
    // This would be implemented to parse literal values only
    // For now, return nullptr
    return nullptr;
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