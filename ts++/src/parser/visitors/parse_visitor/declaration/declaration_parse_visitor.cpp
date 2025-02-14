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

    // Parse storage modifiers (#heap, #stack, #static)
    tokens::TokenType storageClass = tokens::TokenType::ERROR_TOKEN;
    if (check(tokens::TokenType::STACK) || check(tokens::TokenType::HEAP) ||
        check(tokens::TokenType::STATIC)) {
      storageClass = tokens_.peek().getType();
      tokens_.advance();
    }

    // Parse attributes and modifiers (#inline, etc.)
    std::vector<nodes::AttributePtr> attributes;
    while (check(tokens::TokenType::INLINE) ||
           check(tokens::TokenType::ATTRIBUTE)) {
      if (auto attr = parseAttribute()) {
        attributes.push_back(std::move(attr));
      }
    }

    // Parse the declaration keyword (let/const)
    if (match(tokens::TokenType::LET) || match(tokens::TokenType::CONST)) {
      bool isConst = tokens_.previous().getType() == tokens::TokenType::CONST;

      // Parse identifier
      if (!match(tokens::TokenType::IDENTIFIER)) {
        error("Expected variable name");
        return nullptr;
      }
      auto name = tokens_.previous().getLexeme();

      // Parse optional type annotation
      nodes::TypePtr type;
      if (match(tokens::TokenType::COLON)) {
        type = parseType();
        if (!type)
          return nullptr;
      }

      // Parse initializer
      nodes::ExpressionPtr initializer;
      if (match(tokens::TokenType::EQUALS)) {
        initializer = exprVisitor_.parseExpression();
        if (!initializer)
          return nullptr;
      } else if (isConst) {
        error("Const declarations must have an initializer");
        return nullptr;
      }

      if (!consume(tokens::TokenType::SEMICOLON,
                   "Expected ';' after variable declaration")) {
        return nullptr;
      }

      // Create variable declaration node
      auto result = std::make_shared<nodes::VarDeclNode>(
          name, type, initializer, storageClass, isConst, location);

      // Add all parsed attributes
      for (auto &attr : attributes) {
        result->addAttribute(std::move(attr));
      }

      return result;
    }

    error("Expected declaration");
    return nullptr;
  } catch (const std::exception &e) {
    error(std::string("Error parsing declaration: ") + e.what());
    return nullptr;
  }
}

nodes::TypePtr DeclarationParseVisitor::parseType() {
  auto location = tokens_.peek().getLocation();

  // Handle smart pointer types first since they start with #
  if (check(tokens::TokenType::SHARED) || check(tokens::TokenType::UNIQUE) ||
      check(tokens::TokenType::WEAK)) {
    return parseSmartPointerType(location);
  }

  // Handle template types like Vector<T>
  if (check(tokens::TokenType::IDENTIFIER) &&
      tokens_.peekNext().getType() == tokens::TokenType::LESS) {
    return parseTemplateType(location);
  }

  // Parse the base type
  nodes::TypePtr type = parsePrimaryType();
  if (!type)
    return nullptr;

  // Keep parsing modifiers as long as possible
  while (true) {
    if (match(tokens::TokenType::AT)) {
      type = parsePointerType(type, location);
      if (!type)
        return nullptr;
    } else if (match(tokens::TokenType::LEFT_BRACKET)) {
      type = parseArrayType(type, location);
      if (!type)
        return nullptr;
    } else if (match(tokens::TokenType::AMPERSAND)) {
      type = std::make_shared<nodes::ReferenceTypeNode>(type, location);
    } else if (match(tokens::TokenType::PIPE)) {
      type = parseUnionType(type, location);
      if (!type)
        return nullptr;
    } else if (check(tokens::TokenType::IDENTIFIER) &&
               tokens_.peekNext().getType() == tokens::TokenType::LESS) {
      type = parseTemplateType(location);
      if (!type)
        return nullptr;
    } else {
      break;
    }
  }

  return type;
}

nodes::TypePtr DeclarationParseVisitor::parseSmartPointerType(
    const core::SourceLocation &location) {
  // Get the smart pointer kind token and advance
  auto kind = tokens_.peek().getType();
  tokens_.advance();

  if (!consume(tokens::TokenType::LESS,
               "Expected '<' after smart pointer type")) {
    return nullptr;
  }

  // Parse the type argument
  auto pointeeType = parseType(); // Changed from parsePrimaryType to parseType
  if (!pointeeType)
    return nullptr;

  if (!consume(tokens::TokenType::GREATER,
               "Expected '>' after smart pointer type")) {
    return nullptr;
  }

  auto spKind = (kind == tokens::TokenType::SHARED)
                    ? nodes::SmartPointerTypeNode::SmartPointerKind::Shared
                : (kind == tokens::TokenType::UNIQUE)
                    ? nodes::SmartPointerTypeNode::SmartPointerKind::Unique
                    : nodes::SmartPointerTypeNode::SmartPointerKind::Weak;

  return std::make_shared<nodes::SmartPointerTypeNode>(pointeeType, spKind,
                                                       location);
}

nodes::TypePtr DeclarationParseVisitor::parsePrimaryType() {
  auto location = tokens_.peek().getLocation();

  if (tokens::TokenType::TYPE_BEGIN <= tokens_.peek().getType() &&
      tokens_.peek().getType() <= tokens::TokenType::TYPE_END) {
    auto type = tokens_.peek().getType();
    tokens_.advance();
    return std::make_shared<nodes::PrimitiveTypeNode>(type, location);
  } else if (check(tokens::TokenType::IDENTIFIER)) {
    auto name = tokens_.peek().getLexeme();
    tokens_.advance();
    return std::make_shared<nodes::NamedTypeNode>(name, location);
  }

  error("Expected type name");
  return nullptr;
}

nodes::TypePtr DeclarationParseVisitor::parsePointerType(
    nodes::TypePtr baseType, const core::SourceLocation &location) {
  // Handle pointer modifiers
  nodes::PointerTypeNode::PointerKind kind =
      nodes::PointerTypeNode::PointerKind::Raw;
  nodes::ExpressionPtr alignment;

  if (match(tokens::TokenType::UNSAFE)) {
    kind = nodes::PointerTypeNode::PointerKind::Unsafe;
  } else if (match(tokens::TokenType::ALIGNED)) {
    kind = nodes::PointerTypeNode::PointerKind::Aligned;

    if (!consume(tokens::TokenType::LEFT_PAREN, "Expected '(' after aligned")) {
      return nullptr;
    }

    if (!match(tokens::TokenType::NUMBER)) {
      error("Expected alignment value");
      return nullptr;
    }

    alignment = std::make_shared<nodes::LiteralExpressionNode>(
        tokens_.previous().getLocation(), tokens::TokenType::NUMBER,
        tokens_.previous().getLexeme());

    if (!consume(tokens::TokenType::RIGHT_PAREN,
                 "Expected ')' after alignment value")) {
      return nullptr;
    }
  }

  return std::make_shared<nodes::PointerTypeNode>(baseType, kind, alignment,
                                                  location);
}

nodes::TypePtr
DeclarationParseVisitor::parseArrayType(nodes::TypePtr elementType,
                                        const core::SourceLocation &location) {
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

  return std::make_shared<nodes::ArrayTypeNode>(elementType, sizeExpr,
                                                location);
}

nodes::TypePtr
DeclarationParseVisitor::parseUnionType(nodes::TypePtr leftType,
                                        const core::SourceLocation &location) {
  auto rightType = parsePrimaryType();
  if (!rightType)
    return nullptr;

  return std::make_shared<nodes::UnionTypeNode>(leftType, rightType, location);
}

nodes::TypePtr DeclarationParseVisitor::parseTemplateType(
    const core::SourceLocation &location) {
  auto templateName = tokens_.peek().getLexeme();
  tokens_.advance();

  if (!consume(tokens::TokenType::LESS, "Expected '<' after template name")) {
    return nullptr;
  }

  std::vector<nodes::TypePtr> typeArgs;
  do {
    auto typeArg = parsePrimaryType();
    if (!typeArg)
      return nullptr;
    typeArgs.push_back(std::move(typeArg));
  } while (match(tokens::TokenType::COMMA));

  if (!consume(tokens::TokenType::GREATER,
               "Expected '>' after template arguments")) {
    return nullptr;
  }

  return std::make_shared<nodes::TemplateTypeNode>(
      std::make_shared<nodes::NamedTypeNode>(templateName, location),
      std::move(typeArgs), location);
}

nodes::BlockPtr DeclarationParseVisitor::parseBlock() {
  return stmtVisitor_.parseBlock();
}

// Helper method to parse basic types
// nodes::TypePtr DeclarationParseVisitor::parsePrimaryType() {
//     auto location = tokens_.peek().getLocation();

//     // Handle primitive types
//     if (tokens::TokenType::TYPE_BEGIN <= tokens_.peek().getType() &&
//         tokens_.peek().getType() <= tokens::TokenType::TYPE_END) {
//         auto type = tokens_.peek().getType();
//         tokens_.advance();
//         return std::make_shared<nodes::PrimitiveTypeNode>(type, location);
//     }
//     // Handle user-defined types (identifiers)
//     else if (check(tokens::TokenType::IDENTIFIER)) {
//         auto name = tokens_.peek().getLexeme();
//         tokens_.advance();
//         return std::make_shared<nodes::NamedTypeNode>(name, location);
//     }
//     else {
//         error("Expected type name");
//         return nullptr;
//     }
// }

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