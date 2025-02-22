#pragma once
#include "core/diagnostics/error_reporter.h"
#include "ideclaration_visitor.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/visitors/parse_visitor/expression/iexpression_visitor.h"
#include "parser/visitors/parse_visitor/statement/istatement_visitor.h"
#include "tokens/stream/token_stream.h"
#include <iostream>
#include <ostream>

namespace visitors {

class DeclarationParseVisitor;

class ClassDeclarationVisitor {
public:
  ClassDeclarationVisitor(tokens::TokenStream &tokens,
                          core::ErrorReporter &errorReporter,
                          IDeclarationVisitor &declVisitor,
                          IExpressionVisitor &exprVisitor,
                          IStatementVisitor &stmtVisitor)
      : tokens_(tokens), errorReporter_(errorReporter),
        exprVisitor_(exprVisitor), stmtVisitor_(stmtVisitor),
        declVisitor_(declVisitor) {}

  nodes::DeclPtr
  parseClassDecl(const std::vector<tokens::TokenType> &initialModifiers = {}) {
    auto location = tokens_.peek().getLocation();
    std::vector<tokens::TokenType> modifiers = initialModifiers;

    // Parse 'class' keyword
    if (!match(tokens::TokenType::CLASS)) {
      error("Expected 'class' keyword");
      return nullptr;
    }

    // Parse class name
    if (!match(tokens::TokenType::IDENTIFIER)) {
      error("Expected class name");
      return nullptr;
    }
    auto name = tokens_.previous().getLexeme();

    // Parse inheritance
    nodes::TypePtr baseClass;
    if (match(tokens::TokenType::EXTENDS)) {
      baseClass = declVisitor_.parseType();
      if (!baseClass)
        return nullptr;
    }

    // Parse implemented interfaces
    std::vector<nodes::TypePtr> interfaces;
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

    // Parse members
    std::vector<nodes::DeclPtr> members;
    while (!check(tokens::TokenType::RIGHT_BRACE) && !tokens_.isAtEnd()) {
      if (auto member = parseMemberDecl()) {
        std::cout << "next token: " << tokens_.peek().getLexeme()
                  << ", type: " << static_cast<int>(tokens_.peek().getType())
                  << std::endl;
        members.push_back(std::move(member));
      } else {
        std::cout << "next token: " << tokens_.peek().getLexeme()
                  << ", type: " << static_cast<int>(tokens_.peek().getType())
                  << std::endl;
        synchronize();
      }
    }

    std::cout << "Before final consume - token: "
              << (tokens_.isAtEnd() ? "END" : tokens_.peek().getLexeme())
              << std::endl;
    // Consume the closing brace
    if (!consume(tokens::TokenType::RIGHT_BRACE,
                 "Expected '}' after class body")) {
      return nullptr;
    }

    return std::make_shared<nodes::ClassDeclNode>(
        name, std::move(modifiers), std::move(baseClass), std::move(interfaces),
        std::move(members), location);
  }

private:
  nodes::AttributePtr parseClassAttribute() {
    if (!match(tokens::TokenType::ATTRIBUTE)) {
      error("Expected class attribute");
      return nullptr;
    }

    auto location = tokens_.previous().getLocation();
    auto name = tokens_.previous().getLexeme();

    // Remove '#' prefix
    if (!name.empty() && name[0] == '#') {
      name = name.substr(1);
    }

    // Handle attributes with arguments
    nodes::ExpressionPtr argument;
    if (match(tokens::TokenType::LEFT_PAREN)) {
      // Parse argument expression
      if (!match(tokens::TokenType::NUMBER)) {
        error("Expected attribute argument");
        return nullptr;
      }

      argument = std::make_shared<nodes::LiteralExpressionNode>(
          location, tokens::TokenType::NUMBER, tokens_.previous().getLexeme());

      if (!consume(tokens::TokenType::RIGHT_PAREN,
                   "Expected ')' after attribute argument")) {
        return nullptr;
      }
    }

    return std::make_shared<nodes::AttributeNode>(name, argument, location);
  }

  nodes::DeclPtr parseMemberDecl() {
    // Parse member attributes first
    std::vector<nodes::AttributePtr> attributes;
    while (check(tokens::TokenType::ATTRIBUTE)) {
      if (auto attr = parseClassAttribute()) {
        attributes.push_back(std::move(attr));
      } else {
        return nullptr;
      }
    }

    // Parse access modifier if present
    tokens::TokenType accessModifier = tokens::TokenType::ERROR_TOKEN;
    if (check(tokens::TokenType::PUBLIC) || check(tokens::TokenType::PRIVATE) ||
        check(tokens::TokenType::PROTECTED)) {
      accessModifier = tokens_.peek().getType();
      tokens_.advance();
    }

    // Parse member declaration
    nodes::DeclPtr memberDecl;
    if (check(tokens::TokenType::CONSTRUCTOR)) {
      // Parse constructor
      memberDecl = parseConstructor(accessModifier);
    } else if (check(tokens::TokenType::FUNCTION)) {
      // Parse method
      memberDecl = parseMemberFunction(accessModifier);
    } else if (check(tokens::TokenType::LET) ||
               check(tokens::TokenType::CONST)) {
      // Parse field
      memberDecl = parseMemberField(accessModifier);
    } else {
      error("Expected class member declaration");
      return nullptr;
    }

    // Add attributes to the member declaration if it was successfully parsed
    if (memberDecl) {
      for (const auto &attr : attributes) {
        memberDecl->addAttribute(attr);
      }
    }

    return memberDecl;
  }

  nodes::DeclPtr parseConstructor(tokens::TokenType accessModifier) {
    auto location = tokens_.peek().getLocation();

    // Consume 'constructor' keyword
    if (!consume(tokens::TokenType::CONSTRUCTOR,
                 "Expected 'constructor' keyword")) {
      return nullptr;
    }

    // Parse '('
    if (!consume(tokens::TokenType::LEFT_PAREN,
                 "Expected '(' after 'constructor'")) {
      return nullptr;
    }

    // Parse parameter list
    std::vector<nodes::ParamPtr> parameters;
    if (!check(tokens::TokenType::RIGHT_PAREN)) {
      // Assume declVisitor_ has parseParameterList(...)
      if (!parseParameterList(parameters)) {
        return nullptr;
      }
    }

    // Parse ')'
    if (!consume(tokens::TokenType::RIGHT_PAREN,
                 "Expected ')' after constructor parameters")) {
      return nullptr;
    }

    // Parse constructor body block
    if (!consume(tokens::TokenType::LEFT_BRACE,
                 "Expected '{' to start constructor body")) {
      return nullptr;
    }
    auto body = declVisitor_.parseBlock();
    if (!body) {
      return nullptr;
    }
    if (!consume(tokens::TokenType::RIGHT_BRACE,
                 "Expected '}' after constructor body")) {
      return nullptr;
    }

    // Create a ConstructorDeclNode (or your custom node)
    auto constructorNode = std::make_shared<nodes::ConstructorDeclNode>(
        accessModifier, // e.g. tokens::TokenType::PUBLIC, etc.
        std::move(parameters), std::move(body), location);
    return constructorNode;
  }

  nodes::DeclPtr parseMemberFunction(tokens::TokenType accessModifier) {
    auto location = tokens_.peek().getLocation();

    // We already know check(FUNCTION) was true, but let's fully consume it:
    if (!consume(tokens::TokenType::FUNCTION, "Expected 'function' keyword")) {
      return nullptr;
    }

    // Parse method name
    if (!consume(tokens::TokenType::IDENTIFIER,
                 "Expected method name after 'function'")) {
      return nullptr;
    }
    auto methodName = tokens_.previous().getLexeme();

    // Parse parameter list
    if (!consume(tokens::TokenType::LEFT_PAREN,
                 "Expected '(' after method name")) {
      return nullptr;
    }
    std::vector<nodes::ParamPtr> parameters;
    if (!check(tokens::TokenType::RIGHT_PAREN)) {
      if (!parseParameterList(parameters)) {
        return nullptr;
      }
    }
    if (!consume(tokens::TokenType::RIGHT_PAREN,
                 "Expected ')' after parameters")) {
      return nullptr;
    }

    // Optional return type: e.g.  ": int"
    nodes::TypePtr returnType;
    if (match(tokens::TokenType::COLON)) {
      returnType = declVisitor_.parseType();
      if (!returnType) {
        return nullptr;
      }
    }

    // Optional "throws" clause: e.g.  throws Error, AnotherError
    std::vector<nodes::TypePtr> throwsTypes;
    if (match(tokens::TokenType::THROWS)) {
      do {
        auto thrownType = declVisitor_.parseType();
        if (!thrownType) {
          return nullptr;
        }
        throwsTypes.push_back(std::move(thrownType));
      } while (match(tokens::TokenType::COMMA));
    }

    // We could also parse optional "where" constraints here if your grammar
    // includes that.

    // Parse the method body block
    if (!check(tokens::TokenType::LEFT_BRACE)) {
      error("Expected '{' before method body");
      return nullptr;
    }
    auto body = declVisitor_.parseBlock();
    if (!body) {
      return nullptr;
    }

    // Optional function modifiers (like #inline, #virtual) might have been
    // collected earlier or you can parse them similarly. We'll assume we have
    // none for now:
    std::vector<tokens::TokenType> methodModifiers;

    // Create a MethodDeclNode (or your "FunctionDeclNode" with an access
    // modifier)
    auto methodNode = std::make_shared<nodes::MethodDeclNode>(
        methodName, accessModifier, std::move(parameters),
        std::move(returnType), std::move(throwsTypes),
        std::move(methodModifiers), std::move(body), location);

    return methodNode;
  }

  nodes::DeclPtr parseMemberField(tokens::TokenType accessModifier) {
    auto location = tokens_.peek().getLocation();

    bool isConst = false;
    // We know check(LET) or check(CONST) is true
    if (match(tokens::TokenType::LET)) {
      isConst = false;
    } else if (match(tokens::TokenType::CONST)) {
      isConst = true;
    } else {
      error("Expected 'let' or 'const' in field declaration");
      return nullptr;
    }

    // Field name
    if (!consume(tokens::TokenType::IDENTIFIER, "Expected field name")) {
      return nullptr;
    }
    auto fieldName = tokens_.previous().getLexeme();

    // Optional type annotation: ": Type"
    nodes::TypePtr fieldType;
    if (match(tokens::TokenType::COLON)) {
      fieldType = declVisitor_.parseType();
      if (!fieldType) {
        return nullptr;
      }
    }

    // Optional initializer: "= Expression"
    nodes::ExpressionPtr initializer;
    if (match(tokens::TokenType::EQUALS)) {
      initializer = exprVisitor_.parseExpression();
      if (!initializer) {
        return nullptr;
      }
    }

    // Expect semicolon
    if (!consume(tokens::TokenType::SEMICOLON,
                 "Expected ';' after field declaration")) {
      return nullptr;
    }

    // Create a FieldDeclNode (or reuse VarDeclNode with an access modifier)
    auto fieldNode = std::make_shared<nodes::FieldDeclNode>(
        fieldName, accessModifier, isConst, std::move(fieldType),
        std::move(initializer), location);

    return fieldNode;
  }

  bool parseParameterList(std::vector<nodes::ParamPtr> &outParams) {
    // We'll assume we keep parsing until we either hit a ) or something that
    // indicates we're done. Typically you'd check this in the calling function,
    // e.g. while !check(TokenType::RIGHT_PAREN).

    while (true) {
      // If we see something that’s not an identifier, not "ref", and not
      // "const", we probably have no parameter. That might signal an empty list
      // or end of list. The caller would break or handle an error. Here we just
      // break for safety.
      if (!check(tokens::TokenType::IDENTIFIER) &&
          !check(tokens::TokenType::REF) && !check(tokens::TokenType::CONST)) {
        break;
      }

      // Parse a single parameter
      auto param = parseSingleParameter();
      if (!param) {
        // parseSingleParameter() should report any errors
        // We attempt to sync or return false to indicate error
        synchronize();
        return false;
      }

      outParams.push_back(std::move(param));

      // If there's no comma, we're done with the parameter list
      if (!match(tokens::TokenType::COMMA)) {
        break;
      }
    }

    return true;
  }

  nodes::ParamPtr parseSingleParameter() {
    auto location = tokens_.peek().getLocation();

    // 1) Parse optional parameter modifiers: "ref" or "const"
    bool isRef = false;
    bool isConst = false;

    if (match(tokens::TokenType::REF)) {
      isRef = true;
    }

    if (match(tokens::TokenType::CONST)) {
      isConst = true;
    }

    // 2) Parse IDENTIFIER (parameter name)
    if (!consume(tokens::TokenType::IDENTIFIER, "Expected parameter name")) {
      return nullptr;
    }
    auto paramName = tokens_.previous().getLexeme();

    // 3) Optional type annotation (": Type")
    nodes::TypePtr paramType;
    if (match(tokens::TokenType::COLON)) {
      paramType = declVisitor_.parseType();
      // parseType() is presumably on declVisitor_ or in the same parser
      if (!paramType) {
        return nullptr;
      }
    }

    // 4) Optional default value ("= Expression")
    nodes::ExpressionPtr defaultValue;
    if (match(tokens::TokenType::EQUALS)) {
      defaultValue = exprVisitor_.parseExpression();
      if (!defaultValue) {
        return nullptr;
      }
    }

    // Build a new ParameterNode
    auto paramNode = std::make_shared<nodes::ParameterNode>(
        paramName, std::move(paramType), std::move(defaultValue), isRef,
        isConst, location);

    return paramNode;
  }

  bool isClassModifier(tokens::TokenType type) const {
    return type == tokens::TokenType::ALIGNED ||
           type == tokens::TokenType::PACKED ||
           type == tokens::TokenType::ABSTRACT;
  }

  void synchronize() {
    tokens_.advance();
    while (!tokens_.isAtEnd()) {
      if (tokens_.previous().getType() == tokens::TokenType::SEMICOLON)
        return;

      switch (tokens_.peek().getType()) {
      case tokens::TokenType::CLASS:
      case tokens::TokenType::FUNCTION:
      case tokens::TokenType::LET:
      case tokens::TokenType::CONST:
      case tokens::TokenType::PUBLIC:
      case tokens::TokenType::PRIVATE:
      case tokens::TokenType::PROTECTED:
      case tokens::TokenType::RIGHT_BRACE: // Add this to handle end of class
        return;
      default:
        tokens_.advance();
      }
    }
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
  IStatementVisitor &stmtVisitor_;
  IDeclarationVisitor &declVisitor_;
};

} // namespace visitors