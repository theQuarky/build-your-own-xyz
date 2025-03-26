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

class ClassDeclarationVisitor {
public:
  ClassDeclarationVisitor(tokens::TokenStream &tokens,
                          core::ErrorReporter &errorReporter,
                          IDeclarationVisitor &declVisitor,
                          IExpressionVisitor &exprVisitor,
                          IStatementVisitor &stmtVisitor)
      : tokens_(tokens), errorReporter_(errorReporter),
        declVisitor_(declVisitor), exprVisitor_(exprVisitor),
        stmtVisitor_(stmtVisitor) {}

  /// Main entry: Parse a class declaration (with optional initial modifiers).
  // Update the parseClassDecl method in your class_decl_visitor.h file with
  // these debugging statements

  nodes::DeclPtr
  parseClassDecl(const std::vector<tokens::TokenType> &initialModifiers = {}) {
    auto location = tokens_.peek().getLocation();
    std::vector<tokens::TokenType> modifiers = initialModifiers;

    std::cout << "Starting class declaration parsing" << std::endl;

    // Expect 'class' keyword
    if (!match(tokens::TokenType::CLASS)) {
      error("Expected 'class' keyword");
      return nullptr;
    }

    // Parse class name
    if (!match(tokens::TokenType::IDENTIFIER)) {
      error("Expected class name after 'class'");
      return nullptr;
    }
    auto className = tokens_.previous().getLexeme();
    std::cout << "Parsing class: " << className << std::endl;

    // Parse optional generic parameters
    std::vector<nodes::TypePtr> genericParams;
    if (match(tokens::TokenType::LESS)) {
      if (!parseGenericParams(genericParams)) {
        return nullptr; // parseGenericParams() calls error() on its own
      }
    }

    // Parse optional "extends" (base class)
    nodes::TypePtr baseClass;
    if (match(tokens::TokenType::EXTENDS)) {
      baseClass = declVisitor_.parseType();
      if (!baseClass) {
        error("Expected base class type after 'extends'");
        return nullptr;
      }
    }

    // Parse optional "implements" list
    std::vector<nodes::TypePtr> interfaces;
    if (match(tokens::TokenType::IMPLEMENTS)) {
      do {
        auto ifaceType = declVisitor_.parseType();
        if (!ifaceType) {
          error("Expected interface name after 'implements'");
          return nullptr;
        }
        interfaces.push_back(ifaceType);
      } while (match(tokens::TokenType::COMMA));
    }

    // Expect '{' to start class body
    if (!consume(tokens::TokenType::LEFT_BRACE,
                 "Expected '{' before class body")) {
      return nullptr;
    }
    std::cout << "Starting class body for " << className << std::endl;

    // Parse class members until '}' or end-of-file
    // Update the end of class body parsing in the parseClassDecl method
    // Replace the while loop and the code that follows it with this more robust
    // approach:

    // Parse class members until '}' or end-of-file
    // Replace just the class body parsing loop in parseClassDecl with this
    // simplest version:

    // Parse class members
    std::vector<nodes::DeclPtr> members;
    int memberCount = 0;

    // Simple loop - parse until right brace or EOF
    while (!check(tokens::TokenType::RIGHT_BRACE) && !tokens_.isAtEnd()) {
      std::cout << "Parsing class member #" << memberCount
                << ", token: " << tokens_.peek().getLexeme() << " at line "
                << tokens_.peek().getLocation().getLine() << std::endl;

      // Try to parse a member
      auto member = parseMemberDecl();
      if (member) {
        members.push_back(std::move(member));
        memberCount++;
        std::cout << "Successfully parsed member #" << memberCount << std::endl;
      } else {
        std::cout << "Error parsing member, trying to recover..." << std::endl;
        // Skip tokens until we find something that looks like a member start
        synchronize();

        // If we're now at the end of the class, break out
        if (check(tokens::TokenType::RIGHT_BRACE) || tokens_.isAtEnd()) {
          break;
        }
      }
    }

    std::cout << "End of class body, found " << memberCount << " members"
              << std::endl;

    // Consume the closing brace if present
    if (check(tokens::TokenType::RIGHT_BRACE)) {
      tokens_.advance(); // Advance without error reporting
      std::cout << "Successfully closed class " << className << std::endl;
    } else {
      std::cout << "Warning: Class body ended without closing brace"
                << std::endl;
    }

    // Create and return the class node regardless
    return std::make_shared<nodes::ClassDeclNode>(
        className, std::move(modifiers), std::move(baseClass),
        std::move(interfaces), std::move(members), location);
  }
  nodes::DeclPtr parseMemberDecl() {
    try {
      auto location = tokens_.peek().getLocation();

      // Check for an access modifier
      tokens::TokenType accessModifier = tokens::TokenType::ERROR_TOKEN;
      if (check(tokens::TokenType::PUBLIC) ||
          check(tokens::TokenType::PRIVATE) ||
          check(tokens::TokenType::PROTECTED)) {
        accessModifier = tokens_.peek().getType();
        tokens_.advance(); // consume it
      }

      // Handle different member types
      if (check(tokens::TokenType::CONSTRUCTOR)) {
        return parseConstructor(accessModifier);
      } else if (check(tokens::TokenType::FUNCTION)) {
        return parseMethod(accessModifier);
      } else if (check(tokens::TokenType::LET) ||
                 check(tokens::TokenType::CONST)) {
        return parseField(accessModifier);
      } else if (check(tokens::TokenType::GET)) {
        return parsePropertyGetter(accessModifier);
      } else if (check(tokens::TokenType::SET)) {
        return parsePropertySetter(accessModifier);
      } else {
        error("Expected class member declaration");
        return nullptr;
      }
    } catch (const std::exception &e) {
      error(std::string("Error parsing class member: ") + e.what());
      return nullptr;
    }
  }

  nodes::DeclPtr parseConstructor(tokens::TokenType accessModifier) {
    auto location = tokens_.peek().getLocation();

    if (!consume(tokens::TokenType::CONSTRUCTOR,
                 "Expected 'constructor' keyword")) {
      return nullptr;
    }

    // Parameter list: constructor()
    if (!consume(tokens::TokenType::LEFT_PAREN,
                 "Expected '(' after 'constructor'")) {
      return nullptr;
    }

    std::vector<nodes::ParamPtr> parameters;
    if (!check(tokens::TokenType::RIGHT_PAREN)) {
      if (!parseParameterList(parameters)) {
        return nullptr;
      }
    }

    if (!consume(tokens::TokenType::RIGHT_PAREN,
                 "Expected ')' after constructor parameters")) {
      return nullptr;
    }

    // Parse constructor body block
    if (!check(tokens::TokenType::LEFT_BRACE)) {
      error("Expected '{' before constructor body");
      return nullptr;
    }

    auto body = stmtVisitor_.parseBlock();
    if (!body) {
      return nullptr;
    }

    // Build a ConstructorDeclNode
    return std::make_shared<nodes::ConstructorDeclNode>(
        accessModifier, std::move(parameters), std::move(body), location);
  }

  nodes::DeclPtr parseMethod(tokens::TokenType accessModifier) {
    auto location = tokens_.peek().getLocation();

    if (!consume(tokens::TokenType::FUNCTION, "Expected 'function' keyword")) {
      return nullptr;
    }

    // method name
    if (!match(tokens::TokenType::IDENTIFIER)) {
      error("Expected method name after 'function'");
      return nullptr;
    }
    auto methodName = tokens_.previous().getLexeme();

    // parameter list
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

    // optional return type:  : int
    nodes::TypePtr returnType;
    if (match(tokens::TokenType::COLON)) {
      returnType = declVisitor_.parseType();
      if (!returnType) {
        return nullptr;
      }
    }

    // optional throws clause
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

    // Parse method body block
    if (!check(tokens::TokenType::LEFT_BRACE)) {
      error("Expected '{' before method body");
      return nullptr;
    }

    auto body = stmtVisitor_.parseBlock();
    if (!body) {
      return nullptr;
    }

    // Build a MethodDeclNode
    std::vector<tokens::TokenType> methodModifiers;
    return std::make_shared<nodes::MethodDeclNode>(
        methodName, accessModifier, std::move(parameters),
        std::move(returnType), std::move(throwsTypes),
        std::move(methodModifiers), std::move(body), location);
  }

  nodes::DeclPtr parseField(tokens::TokenType accessModifier,
                            bool isConst = false) {
    auto location = tokens_.peek().getLocation();

    if (match(tokens::TokenType::LET)) {
      isConst = false;
    } else if (match(tokens::TokenType::CONST)) {
      isConst = true;
    } else {
      error("Expected 'let' or 'const' in field declaration");
      return nullptr;
    }

    // field name
    if (!match(tokens::TokenType::IDENTIFIER)) {
      error("Expected field name");
      return nullptr;
    }
    auto fieldName = tokens_.previous().getLexeme();

    // optional type annotation: ": Type"
    nodes::TypePtr fieldType;
    if (match(tokens::TokenType::COLON)) {
      fieldType = declVisitor_.parseType();
      if (!fieldType) {
        return nullptr;
      }
    }

    // optional initializer: "= expression"
    nodes::ExpressionPtr initializer;
    if (match(tokens::TokenType::EQUALS)) {
      initializer = exprVisitor_.parseExpression();
      if (!initializer) {
        return nullptr;
      }
    }

    // Expect semicolon at the end
    if (!consume(tokens::TokenType::SEMICOLON,
                 "Expected ';' after field declaration")) {
      return nullptr;
    }

    // Build FieldDeclNode
    return std::make_shared<nodes::FieldDeclNode>(
        fieldName, accessModifier, isConst, std::move(fieldType),
        std::move(initializer), location);
  }

  nodes::DeclPtr parsePropertyGetter(tokens::TokenType accessModifier) {
    auto location = tokens_.peek().getLocation();

    if (!consume(tokens::TokenType::GET, "Expected 'get' keyword")) {
      return nullptr;
    }

    // property name
    if (!match(tokens::TokenType::IDENTIFIER)) {
      error("Expected property name after 'get'");
      return nullptr;
    }
    auto propName = tokens_.previous().getLexeme();

    // Optional parameter list (may not be present)
    if (match(tokens::TokenType::LEFT_PAREN)) {
      if (!consume(tokens::TokenType::RIGHT_PAREN,
                   "Expected empty parameter list for getter")) {
        return nullptr;
      }
    }

    // optional return type: e.g.  : int
    nodes::TypePtr returnType;
    if (match(tokens::TokenType::COLON)) {
      returnType = declVisitor_.parseType();
      if (!returnType) {
        return nullptr;
      }
    }

    // Parse property body
    if (!check(tokens::TokenType::LEFT_BRACE)) {
      error("Expected '{' after property getter declaration");
      return nullptr;
    }

    auto body = stmtVisitor_.parseBlock();
    if (!body) {
      return nullptr;
    }

    // Build a property-decl node in "getter" mode
    return std::make_shared<nodes::PropertyDeclNode>(
        propName, accessModifier, nodes::PropertyKind::Getter, returnType, body,
        location);
  }

  // Update this method in your class_decl_visitor.h
  nodes::DeclPtr parsePropertySetter(tokens::TokenType accessModifier) {
    auto location = tokens_.peek().getLocation();

    std::cout << "Starting property setter parsing at line "
              << location.getLine() << std::endl;

    if (!consume(tokens::TokenType::SET, "Expected 'set' keyword")) {
      return nullptr;
    }

    // property name
    if (!match(tokens::TokenType::IDENTIFIER)) {
      error("Expected property name after 'set'");
      return nullptr;
    }
    auto propName = tokens_.previous().getLexeme();
    std::cout << "Parsing setter for property: " << propName << std::endl;

    // Parse parameter for setter (value: Type)
    if (!match(tokens::TokenType::LEFT_PAREN)) {
      error("Expected '(' after property setter name");
      return nullptr;
    }

    // Standard parameter with parentheses
    if (!match(tokens::TokenType::IDENTIFIER)) {
      error("Expected parameter name in setter");
      return nullptr;
    }

    auto paramName = tokens_.previous().getLexeme();
    std::cout << "Parsing setter parameter: " << paramName << std::endl;

    nodes::TypePtr paramType;
    if (match(tokens::TokenType::COLON)) {
      paramType = declVisitor_.parseType();
      if (!paramType) {
        return nullptr;
      }
    }

    if (!consume(tokens::TokenType::RIGHT_PAREN,
                 "Expected ')' after setter parameter")) {
      return nullptr;
    }

    // Parse setter body block
    if (!check(tokens::TokenType::LEFT_BRACE)) {
      error("Expected '{' after setter parameter list");
      return nullptr;
    }

    std::cout << "Parsing setter body" << std::endl;
    auto body = stmtVisitor_.parseBlock();
    if (!body) {
      std::cout << "Failed to parse setter body" << std::endl;
      return nullptr;
    }

    std::cout << "Successfully parsed property setter" << std::endl;

    // Build a property-decl node in "setter" mode
    return std::make_shared<nodes::PropertyDeclNode>(
        propName, accessModifier, nodes::PropertyKind::Setter, paramType, body,
        location);
  }

  bool parseGenericParams(std::vector<nodes::TypePtr> &outParams) {
    // e.g. < T extends SomeType, U & AnotherType >
    do {
      if (!match(tokens::TokenType::IDENTIFIER)) {
        error("Expected generic parameter name");
        return false;
      }
      auto paramName = tokens_.previous().getLexeme();
      auto paramLoc = tokens_.previous().getLocation();

      // optional "extends" Type
      std::vector<nodes::TypePtr> constraints;
      if (match(tokens::TokenType::EXTENDS)) {
        auto baseConstraint = declVisitor_.parseType();
        if (!baseConstraint) {
          return false;
        }
        constraints.push_back(baseConstraint);

        while (match(tokens::TokenType::AMPERSAND)) {
          // intersection type constraints
          auto extraCon = declVisitor_.parseType();
          if (!extraCon) {
            return false;
          }
          constraints.push_back(extraCon);
        }
      }

      // Build the node (in your code, you might have GenericParamNode)
      auto gpNode = std::make_shared<nodes::GenericParamNode>(
          paramName, std::move(constraints), paramLoc);
      outParams.push_back(gpNode);

    } while (match(tokens::TokenType::COMMA));

    if (!consume(tokens::TokenType::GREATER,
                 "Expected '>' after generic parameters")) {
      return false;
    }
    return true;
  }

  bool parseParameterList(std::vector<nodes::ParamPtr> &paramsOut) {
    // loop until we see ')'
    while (!check(tokens::TokenType::RIGHT_PAREN) && !tokens_.isAtEnd()) {
      auto param = parseSingleParameter();
      if (!param) {
        synchronize();
        return false;
      }
      paramsOut.push_back(std::move(param));

      if (!match(tokens::TokenType::COMMA)) {
        break;
      }
    }
    return true;
  }

  nodes::ParamPtr parseSingleParameter() {
    auto loc = tokens_.peek().getLocation();

    bool isRef = false;
    bool isConst = false;

    // optional 'ref' or 'const'
    if (match(tokens::TokenType::REF)) {
      isRef = true;
    }
    if (match(tokens::TokenType::CONST)) {
      isConst = true;
    }

    // parameter name
    if (!match(tokens::TokenType::IDENTIFIER)) {
      error("Expected parameter name");
      return nullptr;
    }
    auto paramName = tokens_.previous().getLexeme();

    // optional type: ": Type"
    nodes::TypePtr paramType;
    if (match(tokens::TokenType::COLON)) {
      paramType = declVisitor_.parseType();
      if (!paramType) {
        return nullptr;
      }
    }

    // optional default value: "= expression"
    nodes::ExpressionPtr defaultValue;
    if (match(tokens::TokenType::EQUALS)) {
      defaultValue = exprVisitor_.parseExpression();
      if (!defaultValue) {
        return nullptr;
      }
    }

    return std::make_shared<nodes::ParameterNode>(
        paramName, std::move(paramType), std::move(defaultValue), isRef,
        isConst, loc);
  }

private:
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

  bool consume(tokens::TokenType type, const std::string &errMsg) {
    if (check(type)) {
      tokens_.advance();
      return true;
    }
    error(errMsg);
    return false;
  }

  void synchronize() {
    tokens_.advance();
    while (!tokens_.isAtEnd()) {
      if (tokens_.previous().getType() == tokens::TokenType::SEMICOLON)
        return;

      switch (tokens_.peek().getType()) {
      case tokens::TokenType::CLASS:
      case tokens::TokenType::FUNCTION:
      case tokens::TokenType::CONSTRUCTOR:
      case tokens::TokenType::LET:
      case tokens::TokenType::CONST:
      case tokens::TokenType::PUBLIC:
      case tokens::TokenType::PRIVATE:
      case tokens::TokenType::PROTECTED:
      case tokens::TokenType::GET:
      case tokens::TokenType::SET:
      case tokens::TokenType::RIGHT_BRACE:
        return;
      default:
        tokens_.advance();
      }
    }
  }

  void error(const std::string &message) {
    errorReporter_.error(tokens_.peek().getLocation(), message);
  }

  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  IDeclarationVisitor &declVisitor_;
  IExpressionVisitor &exprVisitor_;
  IStatementVisitor &stmtVisitor_;
};

} // namespace visitors