#pragma once
#include "parser/ast.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/nodes/type_nodes.h"
#include <iostream>
#include <string>

namespace core {

class ASTPrinter {
private:
  // ANSI Colors for pretty printing
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
    case tokens::TokenType::INT:
      return "INT";
    case tokens::TokenType::FLOAT:
      return "FLOAT";
    case tokens::TokenType::BOOLEAN:
      return "BOOLEAN";
    case tokens::TokenType::STRING:
      return "STRING";
    case tokens::TokenType::VOID:
      return "VOID";
    // ... add other cases as needed
    default:
      return std::to_string(static_cast<int>(type));
    }
  }

  void visitVarDecl(const nodes::VarDeclNode *node) {
    indent();
    std::cout << GREEN << "VarDecl '" << node->getName() << "' "
              << getLocationString(node->getLocation());

    // Print storage class
    switch (node->getStorageClass()) {
    case tokens::TokenType::HEAP:
      std::cout << " #heap";
      break;
    case tokens::TokenType::STACK:
      std::cout << " #stack";
      break;
    case tokens::TokenType::STATIC:
      std::cout << " #static";
      break;
    default:
      break;
    }

    if (node->isConst())
      std::cout << " const";
    std::cout << RESET << "\n";

    // Print attributes if any
    const auto &attributes = node->getAttributes();
    if (!attributes.empty()) {
      indentLevel_++;
      indent();
      std::cout << YELLOW << "Attributes:" << RESET << "\n";
      for (const auto &attr : attributes) {
        indentLevel_++;
        indent();
        std::cout << YELLOW << attr->getName();
        if (attr->getArgument()) {
          std::cout << " (";
          visitExpr(attr->getArgument().get());
          std::cout << ")";
        }
        std::cout << RESET << "\n";
        indentLevel_--;
      }
      indentLevel_--;
    }

    // Print type
    if (node->getType()) {
      indentLevel_++;
      visitType(node->getType().get());
      indentLevel_--;
    }

    // Print initializer
    if (node->getInitializer()) {
      indentLevel_++;
      indent();
      std::cout << YELLOW << "Init:" << RESET << "\n";
      visitExpr(node->getInitializer().get());
      indentLevel_--;
    }
  }

  void visitExpr(const nodes::ExpressionNode *expr) {
    indentLevel_++;
    if (auto literal =
            dynamic_cast<const nodes::LiteralExpressionNode *>(expr)) {
      indent();
      std::cout << YELLOW << "Literal '" << literal->getValue() << "' "
                << getLocationString(literal->getLocation()) << RESET << "\n";
    } else if (auto binary =
                   dynamic_cast<const nodes::BinaryExpressionNode *>(expr)) {
      indent();
      std::cout << YELLOW << "BinaryExpr ("
                << getOperatorString(binary->getExpressionType()) << ") "
                << getLocationString(binary->getLocation()) << RESET << "\n";
      visitExpr(binary->getLeft().get());
      visitExpr(binary->getRight().get());
    } else if (auto ident =
                   dynamic_cast<const nodes::IdentifierExpressionNode *>(
                       expr)) {
      indent();
      std::cout << YELLOW << "Identifier '" << ident->getName() << "' "
                << getLocationString(ident->getLocation()) << RESET << "\n";
    } else {
      indent();
      // Use tokenTypeToString() to convert the expression's type to a string.
      std::cout << YELLOW
                << "Expr: " << tokenTypeToString(expr->getExpressionType())
                << " " << getLocationString(expr->getLocation()) << RESET
                << "\n";
    }
    indentLevel_--;
  }

  void visitType(const nodes::TypeNode *type) {
    indent();
    std::cout << BLUE << "Type: " << type->toString() << RESET << "\n";

    // Optionally print additional details for composite types.
    if (auto arrType = dynamic_cast<const nodes::ArrayTypeNode *>(type)) {
      indentLevel_++;
      indent();
      std::cout << BLUE
                << "Element Type: " << arrType->getElementType()->toString()
                << RESET << "\n";
      if (arrType->getSize()) {
        indent();
        std::cout << BLUE << "Array Size Expression: ";
        // Optionally call visitExpr for more details.
        std::cout << RESET << "\n";
      }
      indentLevel_--;
    } else if (auto ptrType =
                   dynamic_cast<const nodes::PointerTypeNode *>(type)) {
      indentLevel_++;
      indent();
      std::cout << BLUE << "Base Type: " << ptrType->getBaseType()->toString()
                << RESET << "\n";
      indentLevel_--;
    } else if (auto refType =
                   dynamic_cast<const nodes::ReferenceTypeNode *>(type)) {
      indentLevel_++;
      indent();
      std::cout << BLUE
                << "Referenced Type: " << refType->getBaseType()->toString()
                << RESET << "\n";
      indentLevel_--;
    } else if (auto funcType =
                   dynamic_cast<const nodes::FunctionTypeNode *>(type)) {
      indentLevel_++;
      indent();
      std::cout << BLUE
                << "Return Type: " << funcType->getReturnType()->toString()
                << RESET << "\n";
      indent();
      std::cout << BLUE << "Parameter Types: ";
      for (const auto &param : funcType->getParameterTypes()) {
        std::cout << param->toString() << " ";
      }
      std::cout << RESET << "\n";
      indentLevel_--;
    } else if (auto tmplType =
                   dynamic_cast<const nodes::TemplateTypeNode *>(type)) {
      indentLevel_++;
      indent();
      std::cout << BLUE
                << "Base Template: " << tmplType->getBaseType()->toString()
                << RESET << "\n";
      indent();
      std::cout << BLUE << "Template Arguments: ";
      for (const auto &arg : tmplType->getArguments()) {
        std::cout << arg->toString() << " ";
      }
      std::cout << RESET << "\n";
      indentLevel_--;
    } else if (auto spType =
                   dynamic_cast<const nodes::SmartPointerTypeNode *>(type)) {
      indentLevel_++;
      indent();
      std::cout << BLUE
                << "Pointee Type: " << spType->getPointeeType()->toString()
                << RESET << "\n";
      indentLevel_--;
    } else if (auto unionType =
                   dynamic_cast<const nodes::UnionTypeNode *>(type)) {
      indentLevel_++;
      indent();
      std::cout << BLUE << "Left Type: " << unionType->getLeft()->toString()
                << RESET << "\n";
      indent();
      std::cout << BLUE << "Right Type: " << unionType->getRight()->toString()
                << RESET << "\n";
      indentLevel_--;
    }
  }

  static std::string getOperatorString(tokens::TokenType op) {
    switch (op) {
    case tokens::TokenType::PLUS:
      return "+";
    case tokens::TokenType::MINUS:
      return "-";
    case tokens::TokenType::STAR:
      return "*";
    case tokens::TokenType::SLASH:
      return "/";
    default:
      return "unknown";
    }
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
      }
      // Add other node types as needed.
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
    }
  }
};

} // namespace core
