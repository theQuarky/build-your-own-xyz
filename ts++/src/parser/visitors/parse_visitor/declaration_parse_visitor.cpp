#include "declaration_parse_visitor.h"
#include "parser/nodes/type_nodes.h"
#include "tokens/tokens.h"
#include <iostream>

namespace visitors {

DeclarationParseVisitor::DeclarationParseVisitor(
    tokens::TokenStream &tokens, core::ErrorReporter &errorReporter,
    ExpressionParserVisitor &exprVisitor)
    : tokens_(tokens), errorReporter_(errorReporter),
      exprVisitor_(exprVisitor) {}

nodes::DeclPtr DeclarationParseVisitor::parseDeclaration() {
  auto attributes = parseAttributeList();

  nodes::DeclPtr decl;
  if (match(tokens::TokenType::LET) || match(tokens::TokenType::CONST)) {
    bool isConst = tokens_.previous().getType() == tokens::TokenType::CONST;
    decl = parseVarDecl(isConst);
  } else if (match(tokens::TokenType::FUNCTION)) {
    decl = parseFuncDecl();
  } else {
    error("Expected declaration");
    return nullptr;
  }

  if (decl) {
    for (auto &attr : attributes) {
      decl->addAttribute(attr);
    }
  }

  return decl;
}

nodes::DeclPtr DeclarationParseVisitor::parseVarDecl(bool isConst) {
    auto startLocation = tokens_.peek().getLocation();
    auto storageClass = parseStorageClass();

    // Get identifier
    if (!match(tokens::TokenType::IDENTIFIER)) {
        error("Expected variable name");
        return nullptr;
    }
    auto name = tokens_.previous().getLexeme();

    // Handle type annotation
    nodes::TypePtr type;
    if (match(tokens::TokenType::COLON)) {
        type = parseType();
        if (!type) {
            synchronize();  // Error recovery
            return nullptr;
        }
    }

    // Handle initializer
    nodes::ExpressionPtr initializer;
    if (match(tokens::TokenType::EQUALS)) {
        initializer = exprVisitor_.parseExpression();
        if (!initializer) {
            synchronize();  // Error recovery
            return nullptr;
        }
    } else if (isConst) {
        error("Const declarations require initialization");
        return nullptr;
    }

    // Expect semicolon
    if (!match(tokens::TokenType::SEMICOLON)) {
        error("Expected ';' after variable declaration");
        synchronize();
        return nullptr;
    }

    return std::make_shared<nodes::VarDeclNode>(
        name, type, initializer, storageClass, isConst, startLocation);
}

void DeclarationParseVisitor::synchronize() {
    // Advance until we find a synchronization point
    while (!tokens_.isAtEnd()) {
        // Try to synchronize at statement/declaration boundaries
        if (tokens_.previous().getType() == tokens::TokenType::SEMICOLON) {
            return;
        }

        switch (tokens_.peek().getType()) {
            case tokens::TokenType::FUNCTION:
            case tokens::TokenType::LET:
            case tokens::TokenType::CONST:
            case tokens::TokenType::CLASS:
            case tokens::TokenType::IF:
            case tokens::TokenType::WHILE:
            case tokens::TokenType::RETURN:
                return;
            default:
                tokens_.advance();
        }
    }
}

nodes::DeclPtr DeclarationParseVisitor::parseFuncDecl() {
  auto startLocation = tokens_.peek().getLocation();

  if (!match(tokens::TokenType::IDENTIFIER)) {
    error("Expected function name");
    return nullptr;
  }
  auto name = tokens_.previous().getLexeme();

  if (!match(tokens::TokenType::LEFT_PAREN)) {
    error("Expected '(' after function name");
    return nullptr;
  }

  auto params = parseParameterList();

  if (!match(tokens::TokenType::RIGHT_PAREN)) {
    error("Expected ')' after parameters");
    return nullptr;
  }

  nodes::TypePtr returnType;
  if (match(tokens::TokenType::COLON)) {
    returnType = parseType();
    if (!returnType)
      return nullptr;
  }

  bool isAsync = match(tokens::TokenType::ASYNC);

  // For now, function body is not parsed
  nodes::BlockPtr body;

  return std::make_shared<nodes::FunctionDeclNode>(
      name, params, returnType, body, isAsync, startLocation);
}

nodes::TypePtr DeclarationParseVisitor::parseType() {
  auto baseType = parsePrimaryType();
  if (!baseType)
    return nullptr;
  return parseTypeModifiers(baseType);
}

nodes::TypePtr DeclarationParseVisitor::parsePrimaryType() {
  auto startLocation = tokens_.peek().getLocation();

  if (matchAny({tokens::TokenType::VOID, tokens::TokenType::INT,
                tokens::TokenType::FLOAT, tokens::TokenType::BOOLEAN,
                tokens::TokenType::STRING})) {
    return std::make_shared<nodes::PrimitiveTypeNode>(
        tokens_.previous().getType(), startLocation);
  }

  if (match(tokens::TokenType::IDENTIFIER)) {
    auto name = tokens_.previous().getLexeme();
    std::vector<std::string> qualifiers = {name};

    while (match(tokens::TokenType::DOT)) {
      if (!match(tokens::TokenType::IDENTIFIER)) {
        error("Expected identifier after '.'");
        return nullptr;
      }
      qualifiers.push_back(tokens_.previous().getLexeme());
    }

    if (qualifiers.size() > 1) {
      return std::make_shared<nodes::QualifiedTypeNode>(std::move(qualifiers),
                                                        startLocation);
    }
    return std::make_shared<nodes::NamedTypeNode>(std::move(name),
                                                  startLocation);
  }

  error("Expected type");
  return nullptr;
}

nodes::TypePtr
DeclarationParseVisitor::parseTypeModifiers(nodes::TypePtr baseType) {
  auto startLocation = tokens_.peek().getLocation();

  while (true) {
    if (match(tokens::TokenType::LEFT_BRACKET)) {
      nodes::ExpressionPtr size;
      if (!match(tokens::TokenType::RIGHT_BRACKET)) {
        size = exprVisitor_.parseExpression();
        if (!size)
          return nullptr;

        if (!match(tokens::TokenType::RIGHT_BRACKET)) {
          error("Expected ']' after array size");
          return nullptr;
        }
      }
      baseType =
          std::make_shared<nodes::ArrayTypeNode>(baseType, size, startLocation);
    } else if (match(tokens::TokenType::AT)) {
      using PK = nodes::PointerTypeNode::PointerKind;
      PK kind = PK::Raw;
      nodes::ExpressionPtr alignment;

      if (match(tokens::TokenType::IDENTIFIER)) {
        auto modifier = tokens_.previous().getLexeme();
        if (modifier == "unsafe") {
          kind = PK::Unsafe;
        } else if (modifier == "safe") {
          kind = PK::Safe;
        } else if (modifier == "aligned") {
          kind = PK::Aligned;
          if (!match(tokens::TokenType::LEFT_PAREN)) {
            error("Expected '(' after 'aligned'");
            return nullptr;
          }
          alignment = exprVisitor_.parseExpression();
          if (!alignment)
            return nullptr;
          if (!match(tokens::TokenType::RIGHT_PAREN)) {
            error("Expected ')' after alignment value");
            return nullptr;
          }
        } else {
          error("Invalid pointer modifier: " + modifier);
          return nullptr;
        }
      }

      baseType = std::make_shared<nodes::PointerTypeNode>(
          baseType, kind, alignment, startLocation);
    } else {
      break;
    }
  }

  return baseType;
}

std::vector<nodes::ParamPtr> DeclarationParseVisitor::parseParameterList() {
  std::vector<nodes::ParamPtr> params;

  if (check(tokens::TokenType::RIGHT_PAREN)) {
    return params;
  }

  auto param = parseParameter();
  if (!param)
    return params;
  params.push_back(param);

  while (match(tokens::TokenType::COMMA)) {
    param = parseParameter();
    if (!param)
      return params;
    params.push_back(param);
  }

  return params;
}

nodes::ParamPtr DeclarationParseVisitor::parseParameter() {
  auto startLocation = tokens_.peek().getLocation();

  bool isRef = match(tokens::TokenType::REF);
  bool isConst = match(tokens::TokenType::CONST);

  if (!match(tokens::TokenType::IDENTIFIER)) {
    error("Expected parameter name");
    return nullptr;
  }
  auto name = tokens_.previous().getLexeme();

  if (!match(tokens::TokenType::COLON)) {
    error("Expected ':' after parameter name");
    return nullptr;
  }

  auto type = parseType();
  if (!type)
    return nullptr;

  nodes::ExpressionPtr defaultValue;
  if (match(tokens::TokenType::EQUALS)) {
    defaultValue = exprVisitor_.parseExpression();
    if (!defaultValue)
      return nullptr;
  }

  return std::make_shared<nodes::ParameterNode>(name, type, defaultValue, isRef,
                                                isConst, startLocation);
}

std::vector<nodes::AttributePtr> DeclarationParseVisitor::parseAttributeList() {
  std::vector<nodes::AttributePtr> attributes;

  while (match(tokens::TokenType::ATTRIBUTE)) {
    if (auto attr = parseAttribute()) {
      attributes.push_back(attr);
    }
  }

  return attributes;
}

nodes::AttributePtr DeclarationParseVisitor::parseAttribute() {
  auto startLocation = tokens_.previous().getLocation();

  if (!match(tokens::TokenType::IDENTIFIER)) {
    error("Expected attribute name");
    return nullptr;
  }

  auto name = tokens_.previous().getLexeme();
  nodes::ExpressionPtr argument;

  if (match(tokens::TokenType::LEFT_PAREN)) {
    argument = exprVisitor_.parseExpression();
    if (!argument)
      return nullptr;

    if (!match(tokens::TokenType::RIGHT_PAREN)) {
      error("Expected ')' after attribute argument");
      return nullptr;
    }
  }

  return std::make_shared<nodes::AttributeNode>(name, argument, startLocation);
}

tokens::TokenType DeclarationParseVisitor::parseStorageClass() {
  if (!match(tokens::TokenType::ATTRIBUTE)) {
    return tokens::TokenType::ERROR_TOKEN;
  }

  if (!match(tokens::TokenType::IDENTIFIER)) {
    error("Expected storage class identifier");
    return tokens::TokenType::ERROR_TOKEN;
  }

  const auto &lexeme = tokens_.previous().getLexeme();

  if (lexeme == "stack")
    return tokens::TokenType::STACK;
  if (lexeme == "heap")
    return tokens::TokenType::HEAP;
  if (lexeme == "static")
    return tokens::TokenType::STATIC;

  error("Invalid storage class: " + lexeme);
  return tokens::TokenType::ERROR_TOKEN;
}

bool DeclarationParseVisitor::match(tokens::TokenType type) {
  if (check(type)) {
    tokens_.advance();
    return true;
  }
  return false;
}

bool DeclarationParseVisitor::matchAny(
    const std::vector<tokens::TokenType> &types) {
  for (const auto &type : types) {
    if (match(type))
      return true;
  }
  return false;
}

bool DeclarationParseVisitor::check(tokens::TokenType type) const {
  return !tokens_.isAtEnd() && tokens_.peek().getType() == type;
}

void DeclarationParseVisitor::error(const std::string &message) {
  errorReporter_.error(tokens_.peek().getLocation(), message);
}

} // namespace visitors