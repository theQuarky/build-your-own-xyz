#pragma once
#include "core/diagnostics/error_reporter.h"
#include "ideclaration_visitor.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/visitors/parse_visitor/statement/istatement_visitor.h"
#include "tokens/stream/token_stream.h"

namespace visitors {

class DeclarationParseVisitor;
class StatementParseVisitor;

class FunctionDeclarationVisitor {
public:
  FunctionDeclarationVisitor(tokens::TokenStream &tokens,
                             core::ErrorReporter &errorReporter,
                             IDeclarationVisitor &declVisitor,
                             IStatementVisitor &stmtVisitor)
      : tokens_(tokens), errorReporter_(errorReporter),
        declVisitor_(declVisitor), stmtVisitor_(stmtVisitor) {}

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
      returnType = declVisitor_.parseType();
      if (!returnType)
        return nullptr;
    }

    // Parse function body
    nodes::BlockPtr body;
    if (match(tokens::TokenType::LEFT_BRACE)) {
      body = declVisitor_.parseBlock();
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

    auto type = declVisitor_.parseType();
    if (!type)
      return nullptr;

    nodes::ExpressionPtr defaultValue;
    if (match(tokens::TokenType::EQUALS)) {
      // TODO: Parse default value
    }

    return std::make_shared<nodes::ParameterNode>(name, type, defaultValue,
                                                  isRef, isConst, location);
  }

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

  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  IDeclarationVisitor &declVisitor_;
  IStatementVisitor &stmtVisitor_;
  ;
};

} // namespace visitors