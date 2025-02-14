#pragma once
#include "parser/ast.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/nodes/statement_nodes.h"
#include "parser/nodes/type_nodes.h"
#include <iostream>
#include <string>

namespace core {

class ASTPrinter {
private:
  static constexpr const char *RESET = "\033[0m";
  static constexpr const char *RED = "\033[31m";
  static constexpr const char *GREEN = "\033[32m";
  static constexpr const char *YELLOW = "\033[33m";
  static constexpr const char *BLUE = "\033[34m";

  int indentLevel_ = 0;

  void indent() const { std::cout << std::string(indentLevel_ * 2, ' '); }

  std::string getLocationString(const core::SourceLocation &loc) const {
    return "(" + std::to_string(loc.getLine()) + ":" +
           std::to_string(loc.getColumn()) + ")";
  }

  static std::string tokenTypeToString(tokens::TokenType type) {
    switch (type) {
    case tokens::TokenType::PLUS:
      return "+";
    case tokens::TokenType::MINUS:
      return "-";
    case tokens::TokenType::STAR:
      return "*";
    case tokens::TokenType::SLASH:
      return "/";
    case tokens::TokenType::EQUALS:
      return "=";
    case tokens::TokenType::PLUS_EQUALS:
      return "+=";
    case tokens::TokenType::MINUS_EQUALS:
      return "-=";
    case tokens::TokenType::STAR_EQUALS:
      return "*=";
    case tokens::TokenType::SLASH_EQUALS:
      return "/=";
    case tokens::TokenType::STACK:
      return "#stack";
    case tokens::TokenType::HEAP:
      return "#heap";
    case tokens::TokenType::STATIC:
      return "#static";
    case tokens::TokenType::INT:
      return "int";
    case tokens::TokenType::FLOAT:
      return "float";
    case tokens::TokenType::BOOLEAN:
      return "bool";
    case tokens::TokenType::STRING:
      return "string";
    case tokens::TokenType::VOID:
      return "void";
    default:
      return std::to_string(static_cast<int>(type));
    }
  }

  void visitVarDecl(const nodes::VarDeclNode *node) {
    indent();
    std::cout << GREEN << "VarDecl" << RESET << "\n";

    indentLevel_++;

    // Print name
    indent();
    std::cout << "Name: '" << node->getName() << "' "
              << getLocationString(node->getLocation()) << "\n";

    // Print storage class
    indent();
    std::cout << "Storage: ";
    switch (node->getStorageClass()) {
    case tokens::TokenType::HEAP:
      std::cout << "#heap";
      break;
    case tokens::TokenType::STACK:
      std::cout << "#stack";
      break;
    case tokens::TokenType::STATIC:
      std::cout << "#static";
      break;
    default:
      std::cout << "none";
      break;
    }
    std::cout << "\n";

    // Print const qualifier if present
    if (node->isConst()) {
      indent();
      std::cout << "Qualifier: const\n";
    }

    // Print type information
    if (node->getType()) {
      indent();
      std::cout << "Type:\n";
      indentLevel_++;
      visitType(node->getType().get());
      indentLevel_--;
    }

    // Print attributes if any
    const auto &attributes = node->getAttributes();
    if (!attributes.empty()) {
      indent();
      std::cout << "Attributes:\n";
      indentLevel_++;
      for (const auto &attr : attributes) {
        visitAttribute(attr.get());
      }
      indentLevel_--;
    }

    // Print initializer if present
    if (node->getInitializer()) {
      indent();
      std::cout << "Initializer:\n";
      indentLevel_++;
      visitExpr(node->getInitializer().get());
      indentLevel_--;
    }

    indentLevel_--;
  }

  void visitExprStmt(const nodes::ExpressionStmtNode *node) {
    indent();
    std::cout << "ExpressionStatement "
              << getLocationString(node->getLocation()) << "\n";
    indentLevel_++;
    visitExpr(node->getExpression().get());
    indentLevel_--;
  }

  void visitAttribute(const nodes::AttributeNode *node) {
    indent();
    std::cout << "Attribute: " << node->getName();
    if (node->getArgument()) {
      std::cout << " (";
      visitExpr(node->getArgument().get());
      std::cout << ")";
    }
    std::cout << "\n";
  }

  void visitExpr(const nodes::ExpressionNode *expr) {
    if (!expr) {
      indent();
      std::cout << RED << "null-expression" << RESET << "\n";
      return;
    }

    if (auto literal =
            dynamic_cast<const nodes::LiteralExpressionNode *>(expr)) {
      indent();
      std::cout << "Literal: '" << literal->getValue() << "' "
                << getLocationString(literal->getLocation()) << "\n";
    } else if (auto binary =
                   dynamic_cast<const nodes::BinaryExpressionNode *>(expr)) {
      indent();
      std::cout << "BinaryExpression: "
                << tokenTypeToString(binary->getExpressionType()) << " "
                << getLocationString(binary->getLocation()) << "\n";
      indentLevel_++;
      indent();
      std::cout << "Left:\n";
      indentLevel_++;
      visitExpr(binary->getLeft().get());
      indentLevel_--;
      indent();
      std::cout << "Right:\n";
      indentLevel_++;
      visitExpr(binary->getRight().get());
      indentLevel_--;
      indentLevel_--;
    } else if (auto ident =
                   dynamic_cast<const nodes::IdentifierExpressionNode *>(
                       expr)) {
      indent();
      std::cout << "Identifier: '" << ident->getName() << "' "
                << getLocationString(ident->getLocation()) << "\n";
    } else if (auto assign =
                   dynamic_cast<const nodes::AssignmentExpressionNode *>(
                       expr)) {
      indent();
      std::cout << "Assignment: "
                << tokenTypeToString(assign->getExpressionType()) << " "
                << getLocationString(assign->getLocation()) << "\n";
      indentLevel_++;
      indent();
      std::cout << "Target:\n";
      indentLevel_++;
      visitExpr(assign->getTarget().get());
      indentLevel_--;
      indent();
      std::cout << "Value:\n";
      indentLevel_++;
      visitExpr(assign->getValue().get());
      indentLevel_--;
      indentLevel_--;
    } else {
      indent();
      std::cout << "Expression: "
                << tokenTypeToString(expr->getExpressionType()) << " "
                << getLocationString(expr->getLocation()) << "\n";
    }
  }

  void visitType(const nodes::TypeNode *type) {
    if (!type) {
      indent();
      std::cout << RED << "null-type" << RESET << "\n";
      return;
    }

    indent();
    std::cout << type->toString() << "\n";

    indentLevel_++;
    if (auto arrType = dynamic_cast<const nodes::ArrayTypeNode *>(type)) {
      indent();
      std::cout << "ElementType:\n";
      indentLevel_++;
      visitType(arrType->getElementType().get());
      indentLevel_--;
      if (arrType->getSize()) {
        indent();
        std::cout << "Size:\n";
        indentLevel_++;
        visitExpr(arrType->getSize().get());
        indentLevel_--;
      }
    } else if (auto ptrType =
                   dynamic_cast<const nodes::PointerTypeNode *>(type)) {
      indent();
      std::cout << "BaseType:\n";
      indentLevel_++;
      visitType(ptrType->getBaseType().get());
      indentLevel_--;
    }
    indentLevel_--;
  }

public:
  void print(const parser::AST &ast) {
    std::cout << "\nAbstract Syntax Tree:\n" << std::string(80, '-') << "\n";

    const auto &nodes = ast.getNodes();
    if (nodes.empty()) {
      std::cout << RED << "Empty AST" << RESET << "\n";
      return;
    }

    for (const auto &node : nodes) {
      if (auto varDecl = std::dynamic_pointer_cast<nodes::VarDeclNode>(node)) {
        visitVarDecl(varDecl.get());
      } else if (auto exprStmt =
                     std::dynamic_pointer_cast<nodes::ExpressionStmtNode>(
                         node)) {
        visitExprStmt(exprStmt.get());
      } else {
        indent();
        std::cout << RED << "Unknown node type" << RESET << "\n";
      }
    }

    std::cout << std::string(80, '-') << "\n";
  }

  void print(const nodes::NodePtr &node) {
    if (!node) {
      indent();
      std::cout << RED << "nullptr" << RESET << "\n";
      return;
    }

    if (auto varDecl = std::dynamic_pointer_cast<nodes::VarDeclNode>(node)) {
      visitVarDecl(varDecl.get());
    } else if (auto expr =
                   std::dynamic_pointer_cast<nodes::ExpressionNode>(node)) {
      visitExpr(expr.get());
    } else if (auto exprStmt =
                   std::dynamic_pointer_cast<nodes::ExpressionStmtNode>(node)) {
      visitExprStmt(exprStmt.get());
    } else {
      indent();
      std::cout << RED << "Unknown node type" << RESET << "\n";
    }
  }
};

} // namespace core