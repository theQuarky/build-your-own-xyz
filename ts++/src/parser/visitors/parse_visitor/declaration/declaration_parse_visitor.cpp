#include "declaration_parse_visitor.h"
#include "parser/visitors/parse_visitor/expression/iexpression_visitor.h"
#include <cassert>

namespace visitors {

DeclarationParseVisitor::DeclarationParseVisitor(
    tokens::TokenStream &tokens, core::ErrorReporter &errorReporter,
    IExpressionVisitor &exprVisitor, IStatementVisitor &stmtVisitor)
    : tokens_(tokens), errorReporter_(errorReporter), exprVisitor_(exprVisitor),
      stmtVisitor_(stmtVisitor),
      varDeclVisitor_(tokens, errorReporter, exprVisitor, *this),
      funcDeclVisitor_(tokens, errorReporter, *this, stmtVisitor),
      classDeclVisitor_(tokens, errorReporter, *this) {
  // assert(&tokens != nullptr && "Token stream cannot be null");
  // assert(&errorReporter != nullptr && "Error reporter cannot be null");
}

nodes::DeclPtr DeclarationParseVisitor::parseDeclaration() {
  try {
    auto location = tokens_.peek().getLocation();

    // Parse storage class
    tokens::TokenType storageClass = tokens::TokenType::ERROR_TOKEN;
    if (check(tokens::TokenType::STACK) || check(tokens::TokenType::HEAP) ||
        check(tokens::TokenType::STATIC)) {
      storageClass = tokens_.peek().getType();
      tokens_.advance(); // Consume the storage class token
    }
    // Parse attributes
    auto attributes = parseAttributeList();

    // Parse the actual declaration
    nodes::DeclPtr result;
    if (match(tokens::TokenType::LET) || match(tokens::TokenType::CONST)) {
      bool isConst = tokens_.previous().getType() == tokens::TokenType::CONST;
      result = varDeclVisitor_.parseVarDecl(isConst, storageClass);
    } else if (match(tokens::TokenType::FUNCTION)) {
      result = funcDeclVisitor_.parseFuncDecl();
    } else if (match(tokens::TokenType::CLASS)) {
      result = classDeclVisitor_.parseClassDecl();
    } else {
      error("Expected declaration");
      return nullptr;
    }

    // Add attributes to declaration
    if (result) {
      for (auto &attr : attributes) {
        result->addAttribute(std::move(attr));
      }
    }

    return result;
  } catch (const std::exception &e) {
    error(std::string("Error parsing declaration: ") + e.what());
    return nullptr;
  }
}

nodes::TypePtr DeclarationParseVisitor::parseType() {
  auto location = tokens_.peek().getLocation();

  // Parse base type
  nodes::TypePtr baseType;

  // Handle primitive types
  if (tokens::TokenType::TYPE_BEGIN <= tokens_.peek().getType() &&
      tokens_.peek().getType() <= tokens::TokenType::TYPE_END) {
    auto type = tokens_.peek().getType();
    tokens_.advance();
    baseType = std::make_shared<nodes::PrimitiveTypeNode>(type, location);
  }
  // Handle user-defined types (identifiers)
  else if (check(tokens::TokenType::IDENTIFIER)) {
    auto name = tokens_.peek().getLexeme();
    tokens_.advance();
    baseType = std::make_shared<nodes::NamedTypeNode>(name, location);
  } else {
    error("Expected type name");
    return nullptr;
  }

  // Check for array type
  if (match(tokens::TokenType::LEFT_BRACKET)) {
    // Parse optional size expression
    nodes::ExpressionPtr sizeExpr;
    if (!check(tokens::TokenType::RIGHT_BRACKET)) {
      sizeExpr = exprVisitor_.parseExpression();
      if (!sizeExpr)
        return nullptr;
    }

    if (!consume(tokens::TokenType::RIGHT_BRACKET,
                 "Expected ']' after array type")) {
      return nullptr;
    }

    // Create array type node
    return std::make_shared<nodes::ArrayTypeNode>(baseType, sizeExpr, location);
  }

  return baseType;
}

nodes::BlockPtr DeclarationParseVisitor::parseBlock() {
  return stmtVisitor_.parseBlock();
}

tokens::TokenType DeclarationParseVisitor::parseStorageClass() {
  if (!check(tokens::TokenType::ATTRIBUTE)) {
    return tokens::TokenType::ERROR_TOKEN;
  }

  std::string lexeme = tokens_.peek().getLexeme();
  tokens_.advance();

  if (lexeme == "#stack")
    return tokens::TokenType::STACK;
  if (lexeme == "#heap")
    return tokens::TokenType::HEAP;
  if (lexeme == "#static")
    return tokens::TokenType::STATIC;

  error("Invalid storage class: " + lexeme);
  return tokens::TokenType::ERROR_TOKEN;
}

std::vector<nodes::AttributePtr> DeclarationParseVisitor::parseAttributeList() {
  std::vector<nodes::AttributePtr> attributes;

  while (check(tokens::TokenType::ATTRIBUTE)) {
    // Skip storage class attributes
    if (tokens_.peek().getType() == tokens::TokenType::STACK ||
        tokens_.peek().getType() == tokens::TokenType::HEAP ||
        tokens_.peek().getType() == tokens::TokenType::STATIC) {
      break;
    }

    match(tokens::TokenType::ATTRIBUTE);
    if (auto attr = parseAttribute()) {
      attributes.push_back(attr);
    }
  }
  return attributes;
}
nodes::AttributePtr DeclarationParseVisitor::parseAttribute() {
  auto location = tokens_.previous().getLocation();
  std::string lexeme = tokens_.previous().getLexeme();

  // Remove '#' prefix if present
  if (!lexeme.empty() && lexeme[0] == '#') {
    lexeme = lexeme.substr(1);
  } else {
    error("Expected attribute prefix '#'");
    return nullptr;
  }

  // Parse attribute argument if present
  nodes::ExpressionPtr argument;
  if (match(tokens::TokenType::LEFT_PAREN)) {
    argument = exprVisitor_.parseExpression();
    if (!argument)
      return nullptr;

    if (!consume(tokens::TokenType::RIGHT_PAREN,
                 "Expected ')' after attribute argument")) {
      return nullptr;
    }
  }

  return std::make_shared<nodes::AttributeNode>(lexeme, argument, location);
}

bool DeclarationParseVisitor::consume(tokens::TokenType type,
                                      const std::string &message) {
  if (check(type)) {
    tokens_.advance();
    return true;
  }
  error(message);
  return false;
}

bool DeclarationParseVisitor::match(tokens::TokenType type) {
  if (check(type)) {
    tokens_.advance();
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