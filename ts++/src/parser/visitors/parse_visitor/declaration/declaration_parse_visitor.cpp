#include "declaration_parse_visitor.h"
#include "parser/nodes/declaration_nodes.h"
#include <iostream>

namespace visitors {

DeclarationParseVisitor::DeclarationParseVisitor(
    tokens::TokenStream &tokens, core::ErrorReporter &errorReporter,
    ExpressionParseVisitor &exprVisitor, StatementParseVisitor &stmtVisitor)
    : tokens_(tokens), errorReporter_(errorReporter), exprVisitor_(exprVisitor),
      stmtVisitor_(stmtVisitor), varDeclVisitor_(tokens, errorReporter),
      funcDeclVisitor_(tokens, errorReporter),
      classDeclVisitor_(tokens, errorReporter) {

  // Setup callbacks
  varDeclVisitor_.setCallbacks(
      [this]() { return exprVisitor_.parseExpression(); },
      [this]() { return parseType(); });

  funcDeclVisitor_.setCallbacks([this]() { return parseBlock(); },
                                [this]() { return parseType(); });

  classDeclVisitor_.setCallbacks([this]() { return parseDeclaration(); },
                                 [this]() { return parseType(); });
}

nodes::DeclPtr DeclarationParseVisitor::parseDeclaration() {
  try {

    // Parse attributes first
    auto attributes = parseAttributeList();

    // Parse storage class
    tokens::TokenType storageClass = tokens::TokenType::ERROR_TOKEN;
    auto nextLexeme = tokens_.peek().getLexeme();
    if (nextLexeme == "#stack" || nextLexeme == "#heap" ||
        nextLexeme == "#static") {
      storageClass = parseStorageClass();
      if (storageClass == tokens::TokenType::ERROR_TOKEN) {
        return nullptr;
      }
    }

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
    synchronize();
    return nullptr;
  }
}

nodes::TypePtr DeclarationParseVisitor::parseType() {
  // First, parse a primary type.
  auto left = parsePrimaryType();
  if (!left)
    return nullptr;

  // Now, check for union operator(s) by looking at the token lexeme.
  while (!tokens_.isAtEnd() && tokens_.peek().getLexeme() == "|") {
    // Consume the '|' token.
    tokens_.advance();

    // Parse the type after '|'
    auto right = parsePrimaryType();
    if (!right) {
      error("Expected type after '|'");
      return nullptr;
    }
    // Build a union type node combining left and right.
    left = std::make_shared<nodes::UnionTypeNode>(left, right,
                                                  left->getLocation());
  }

  // Finally, apply any type modifiers (like arrays or pointers) to the entire
  // union type.
  return parseTypeModifiers(left);
}

nodes::TypePtr DeclarationParseVisitor::parsePrimaryType() {
  auto location = tokens_.peek().getLocation();
  if (!tokens_.isAtEnd()) {
    std::string lex = tokens_.peek().getLexeme();
    if (lex == "#shared" || lex == "#unique" || lex == "#weak") {
      // Consume the smart pointer token.
      auto spToken = tokens_.peek();
      tokens_.advance();
      // Expect a '<' after the smart pointer keyword.
      if (!match(tokens::TokenType::LESS)) {
        error("Expected '<' after " + lex);
        return nullptr;
      }
      // Parse the pointee type.
      auto pointee = parseType();
      if (!pointee)
        return nullptr;
      // Expect a '>' after the type.
      if (!match(tokens::TokenType::GREATER)) {
        error("Expected '>' after smart pointer type");
        return nullptr;
      }
      // Determine the smart pointer kind.
      nodes::SmartPointerTypeNode::SmartPointerKind kind;
      if (lex == "#shared")
        kind = nodes::SmartPointerTypeNode::SmartPointerKind::Shared;
      else if (lex == "#unique")
        kind = nodes::SmartPointerTypeNode::SmartPointerKind::Unique;
      else if (lex == "#weak")
        kind = nodes::SmartPointerTypeNode::SmartPointerKind::Weak;
      else {
        error("Unknown smart pointer type: " + lex);
        return nullptr;
      }
      return std::make_shared<nodes::SmartPointerTypeNode>(pointee, kind,
                                                           location);
    }
  }

  // Branch for primitive type
  if (matchAny({tokens::TokenType::VOID, tokens::TokenType::INT,
                tokens::TokenType::FLOAT, tokens::TokenType::BOOLEAN,
                tokens::TokenType::STRING})) {
    return std::make_shared<nodes::PrimitiveTypeNode>(
        tokens_.previous().getType(), location);
  }

  if (match(tokens::TokenType::IDENTIFIER)) {
    auto name = tokens_.previous().getLexeme();
    std::vector<std::string> qualifiers = {name};
    auto baseType =
        std::make_shared<nodes::NamedTypeNode>(std::move(name), location);
    if (match(tokens::TokenType::LESS)) { // Expect '<'
      std::vector<nodes::TypePtr> arguments;

      // Parse the first type argument.
      auto arg = parseType();
      if (!arg) {
        error("Expected type in template parameter list");
        return nullptr;
      }
      arguments.push_back(arg);

      // Parse additional type arguments separated by commas.
      while (match(tokens::TokenType::COMMA)) {
        arg = parseType();
        if (!arg) {
          error("Expected type in template parameter list");
          return nullptr;
        }
        arguments.push_back(arg);
      }

      if (!match(tokens::TokenType::GREATER)) { // Expect '>'
        error("Expected '>' after template parameter list");
        return nullptr;
      }

      return std::make_shared<nodes::TemplateTypeNode>(baseType, arguments,
                                                       location);
    }
    while (match(tokens::TokenType::DOT)) {
      if (!match(tokens::TokenType::IDENTIFIER)) {
        error("Expected identifier after '.'");
        return nullptr;
      }
      qualifiers.push_back(tokens_.previous().getLexeme());
    }

    if (qualifiers.size() > 1) {
      return std::make_shared<nodes::QualifiedTypeNode>(std::move(qualifiers),
                                                        location);
    }
    return std::make_shared<nodes::NamedTypeNode>(std::move(name), location);
  }

  error("Expected type");
  return nullptr;
}

nodes::TypePtr
DeclarationParseVisitor::parseTypeModifiers(nodes::TypePtr baseType) {
  auto location = tokens_.peek().getLocation();

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
          std::make_shared<nodes::ArrayTypeNode>(baseType, size, location);
    } else if (match(tokens::TokenType::AT)) {
      using PK = nodes::PointerTypeNode::PointerKind;
      PK kind = PK::Raw;
      nodes::ExpressionPtr alignExpr;

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
          alignExpr = exprVisitor_.parseExpression();
          if (!alignExpr)
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

      baseType = std::make_shared<nodes::PointerTypeNode>(baseType, kind,
                                                          alignExpr, location);
    } else if (match(tokens::TokenType::AMPERSAND)) { // Make sure AMPERSAND is
                                                      // defined in your token
                                                      // enum.
      baseType = std::make_shared<nodes::ReferenceTypeNode>(baseType, location);
    } else {
      break;
    }
  }

  return baseType;
}

nodes::BlockPtr DeclarationParseVisitor::parseBlock() {
  return stmtVisitor_.parseBlock();
}

tokens::TokenType DeclarationParseVisitor::parseStorageClass() {
  if (!tokens_.check(tokens::TokenType::ATTRIBUTE)) {
    return tokens::TokenType::ERROR_TOKEN;
  }

  auto lexeme = tokens_.peek().getLexeme();
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

  // Loop while the next token is an attribute token but not a storage class
  // token.
  while (check(tokens::TokenType::ATTRIBUTE)) {
    // Peek at the token lexeme
    std::string lexeme = tokens_.peek().getLexeme();
    // If it's a storage class token, break out of the loop
    if (lexeme == "#stack" || lexeme == "#heap" || lexeme == "#static") {
      break;
    }
    // Otherwise, consume the ATTRIBUTE token and parse the attribute.
    match(tokens::TokenType::ATTRIBUTE);
    if (auto attr = parseAttribute()) {
      attributes.push_back(attr);
    }
  }
  return attributes;
}

nodes::AttributePtr DeclarationParseVisitor::parseAttribute() {
  auto location =
      tokens_.previous().getLocation(); // location of the attribute token

  // Get the attribute token's lexeme (e.g., "#inline" or "#optimize")
  std::string lex = tokens_.previous().getLexeme();
  // If the lexeme starts with '#' remove it.
  if (!lex.empty() && lex[0] == '#') {
    lex = lex.substr(1);
  } else {
    error("Expected attribute prefix '#'");
    return nullptr;
  }

  // For attributes that have an argument, check for '('.
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

  return std::make_shared<nodes::AttributeNode>(lex, argument, location);
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

bool DeclarationParseVisitor::consume(tokens::TokenType type,
                                      const std::string &message) {
  if (check(type)) {
    tokens_.advance();
    return true;
  }
  error(message);
  return false;
}

void DeclarationParseVisitor::error(const std::string &message) {
  errorReporter_.error(tokens_.peek().getLocation(), message);
}

void DeclarationParseVisitor::synchronize() {
  tokens_.advance();

  while (!tokens_.isAtEnd()) {
    switch (tokens_.peek().getType()) {
    case tokens::TokenType::CLASS:
    case tokens::TokenType::FUNCTION:
    case tokens::TokenType::LET:
    case tokens::TokenType::CONST:
    case tokens::TokenType::RETURN:
      return;
    default:
      break;
    }

    tokens_.advance();
  }
}

} // namespace visitors