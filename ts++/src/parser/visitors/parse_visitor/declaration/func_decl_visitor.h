#pragma once
#include "core/diagnostics/error_reporter.h"
#include "ideclaration_visitor.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/visitors/parse_visitor/expression/iexpression_visitor.h"
#include "parser/visitors/parse_visitor/statement/istatement_visitor.h"
#include "tokens/stream/token_stream.h"
#include <iostream>
#include <iterator>
#include <ostream>

namespace visitors {

class DeclarationParseVisitor;
class StatementParseVisitor;

class FunctionDeclarationVisitor {
public:
  FunctionDeclarationVisitor(tokens::TokenStream &tokens,
                             core::ErrorReporter &errorReporter,
                             IExpressionVisitor &exprVisitor,
                             IDeclarationVisitor &declVisitor,
                             IStatementVisitor &stmtVisitor)
      : tokens_(tokens), errorReporter_(errorReporter),
        exprVisitor_(exprVisitor), declVisitor_(declVisitor),
        stmtVisitor_(stmtVisitor) {}

  nodes::DeclPtr parseFuncDecl() {
    auto location = tokens_.peek().getLocation();

    // Parse 'function' keyword
    if (!match(tokens::TokenType::FUNCTION)) {
      error("Expected 'function' keyword");
      return nullptr;
    }

    // Parse function name
    if (!match(tokens::TokenType::IDENTIFIER)) {
      error("Expected function name");
      return nullptr;
    }
    auto name = tokens_.previous().getLexeme();

    // Parse parameter list
    if (!consume(tokens::TokenType::LEFT_PAREN,
                 "Expected '(' after function name")) {
      return nullptr;
    }

    std::vector<nodes::ParamPtr> parameters;

    // Parse parameters if there are any
    if (!check(tokens::TokenType::RIGHT_PAREN)) {
      // Parse first parameter
      auto param = parseParameter();
      if (!param)
        return nullptr;
      parameters.push_back(std::move(param));

      // Parse additional parameters
      std::cout << "current token: " << tokens_.getCurrentToken().getLexeme()
                << std::endl;
      while (match(tokens::TokenType::COMMA)) {
        std::cout << "checkpoint a:" << std::endl;
        if (check(tokens::TokenType::RIGHT_PAREN)) {
          error("Expected parameter after ','");
          return nullptr;
        }
        std::cout << "checkpoint b:" << std::endl;
        param = parseParameter();
        if (!param)
          return nullptr;
        std::cout << "checkpoint c:" << std::endl;
        parameters.push_back(std::move(param));
      }
    }

    if (!consume(tokens::TokenType::RIGHT_PAREN,
                 "Expected ')' after parameters")) {
      return nullptr;
    }

    // Parse return type
    nodes::TypePtr returnType;
    if (match(tokens::TokenType::COLON)) {
      returnType = declVisitor_.parseType();
      if (!returnType) {
        return nullptr;
      }
    }

    // Parse function body
    if (!match(tokens::TokenType::LEFT_BRACE)) {
      error("Expected '{' before function body");
      return nullptr;
    }

    auto body = stmtVisitor_.parseBlock();
    if (!body) {
      return nullptr;
    }

    // Create and return function declaration node
    return std::make_shared<nodes::FunctionDeclNode>(
        name, std::move(parameters), std::move(returnType), std::move(body),
        false, // isAsync
        location);
  }

private:
  nodes::ParamPtr parseParameter() {
    auto location = tokens_.peek().getLocation();

    // Parse parameter name
    if (!match(tokens::TokenType::IDENTIFIER)) {
      error("Expected parameter name");
      return nullptr;
    }
    auto name = tokens_.previous().getLexeme();

    // Parse parameter type
    if (!consume(tokens::TokenType::COLON,
                 "Expected ':' after parameter name")) {
      return nullptr;
    }

    auto type = declVisitor_.parseType();
    if (!type) {
      return nullptr;
    }

    // Create parameter node
    return std::make_shared<nodes::ParameterNode>(name, std::move(type),
                                                  nullptr, // default value
                                                  false,   // isRef
                                                  false,   // isConst
                                                  location);
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

  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  IExpressionVisitor &exprVisitor_;
  IDeclarationVisitor &declVisitor_;
  IStatementVisitor &stmtVisitor_;
};

} // namespace visitors