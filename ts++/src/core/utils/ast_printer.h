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

  // Function declaration visitor
  void visitFuncDecl(const nodes::FunctionDeclNode *node) {
    indent();
    std::cout << BLUE << "FunctionDecl" << RESET << " "
              << getLocationString(node->getLocation()) << "\n";

    indentLevel_++;

    // Print modifiers - Add this section
    const auto &modifiers = node->getModifiers();
    if (!modifiers.empty()) {
      indent();
      std::cout << "Modifiers:\n";
      indentLevel_++;
      for (const auto &modifier : modifiers) {
        indent();
        std::cout << modifierToString(modifier) << "\n";
      }
      indentLevel_--;
    }

    // Print function name
    indent();
    std::cout << "Name: '" << node->getName() << "'\n";

    // Print generic parameters and constraints if this is a generic function
    if (auto genericFunc =
            dynamic_cast<const nodes::GenericFunctionDeclNode *>(node)) {
      // Print generic type parameters
      if (!genericFunc->getGenericParams().empty()) {
        indent();
        std::cout << "Generic Parameters:\n";
        indentLevel_++;
        for (const auto &param : genericFunc->getGenericParams()) {
          indent();
          std::cout << param->toString() << "\n";
        }
        indentLevel_--;
      }

      // Print where constraints
      if (!genericFunc->getConstraints().empty()) {
        indent();
        std::cout << "Constraints:\n";
        indentLevel_++;
        for (const auto &[paramName, constraint] :
             genericFunc->getConstraints()) {
          indent();
          std::cout << paramName << ": " << constraint->toString() << "\n";
        }
        indentLevel_--;
      }
    }

    // Print function parameters
    indent();
    std::cout << "Parameters:\n";
    indentLevel_++;
    for (const auto &param : node->getParameters()) {
      visitParameter(param.get());
    }
    indentLevel_--;

    // Print return type
    if (node->getReturnType()) {
      indent();
      std::cout << "Return Type:\n";
      indentLevel_++;
      visitType(node->getReturnType().get());
      indentLevel_--;
    }

    if (!node->getThrowsTypes().empty()) {
      indent();
      std::cout << "Throws:\n";
      indentLevel_++;
      for (const auto &throwType : node->getThrowsTypes()) {
        indent();
        visitType(throwType.get());
      }
      indentLevel_--;
    }

    // Print function body
    if (node->getBody()) {
      indent();
      std::cout << "Body:\n";
      indentLevel_++;
      visitBlock(node->getBody().get());
      indentLevel_--;
    }

    // Print async status
    if (node->isAsync()) {
      indent();
      std::cout << "Async: true\n";
    }

    indentLevel_--;
  }

  // Parameter visitor
  void visitParameter(const nodes::ParameterNode *node) {
    indent();
    std::cout << YELLOW << "Parameter" << RESET << " '" << node->getName()
              << "' " << getLocationString(node->getLocation()) << "\n";

    indentLevel_++;

    // Print parameter type
    if (node->getType()) {
      indent();
      std::cout << "Type:\n";
      indentLevel_++;
      visitType(node->getType().get());
      indentLevel_--;
    }

    // Print modifiers
    if (node->isRef()) {
      indent();
      std::cout << "Modifier: ref\n";
    }
    if (node->isConst()) {
      indent();
      std::cout << "Modifier: const\n";
    }

    // Print default value if present
    if (node->getDefaultValue()) {
      indent();
      std::cout << "Default Value:\n";
      indentLevel_++;
      visitExpr(node->getDefaultValue().get());
      indentLevel_--;
    }

    indentLevel_--;
  }

  // Block visitor
  void visitBlock(const nodes::BlockNode *node) {
    indent();
    std::cout << "Block " << getLocationString(node->getLocation()) << "\n";

    indentLevel_++;
    for (const auto &stmt : node->getStatements()) {
      visitStmt(stmt.get());
    }
    indentLevel_--;
  }

  // Statement visitor
  void visitStmt(const nodes::StatementNode *stmt) {
    if (!stmt) {
      indent();
      std::cout << RED << "null-statement" << RESET << "\n";
      return;
    }

    if (auto exprStmt = dynamic_cast<const nodes::ExpressionStmtNode *>(stmt)) {
      visitExprStmt(exprStmt);
    } else if (auto returnStmt =
                   dynamic_cast<const nodes::ReturnStmtNode *>(stmt)) {
      visitReturnStmt(returnStmt);
    } else if (auto ifStmt = dynamic_cast<const nodes::IfStmtNode *>(stmt)) {
      visitIfStmt(ifStmt);
    } else if (auto declStmt =
                   dynamic_cast<const nodes::DeclarationStmtNode *>(stmt)) {
      visitDeclStmt(declStmt);
    } else {
      indent();
      std::cout << RED << "Unknown statement type" << RESET << "\n";
    }
  }

  // Return statement visitor
  void visitReturnStmt(const nodes::ReturnStmtNode *node) {
    indent();
    std::cout << "Return " << getLocationString(node->getLocation()) << "\n";

    if (node->getValue()) {
      indentLevel_++;
      visitExpr(node->getValue().get());
      indentLevel_--;
    }
  }

  // If statement visitor
  void visitIfStmt(const nodes::IfStmtNode *node) {
    indent();
    std::cout << "If " << getLocationString(node->getLocation()) << "\n";

    indentLevel_++;

    indent();
    std::cout << "Condition:\n";
    indentLevel_++;
    visitExpr(node->getCondition().get());
    indentLevel_--;

    indent();
    std::cout << "Then:\n";
    indentLevel_++;
    visitStmt(node->getThenBranch().get());
    indentLevel_--;

    if (node->getElseBranch()) {
      indent();
      std::cout << "Else:\n";
      indentLevel_++;
      visitStmt(node->getElseBranch().get());
      indentLevel_--;
    }

    indentLevel_--;
  }

  // Declaration statement visitor
  void visitDeclStmt(const nodes::DeclarationStmtNode *node) {
    indent();
    std::cout << "Declaration Statement "
              << getLocationString(node->getLocation()) << "\n";

    indentLevel_++;
    if (auto varDecl = std::dynamic_pointer_cast<nodes::VarDeclNode>(
            node->getDeclaration())) {
      visitVarDecl(varDecl.get());
    } else {
      std::cout << RED << "Unknown declaration type" << RESET << "\n";
    }
    indentLevel_--;
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

  std::string modifierToString(tokens::TokenType modifier) {
    switch (modifier) {
    case tokens::TokenType::INLINE:
      return "#inline";
    case tokens::TokenType::VIRTUAL:
      return "#virtual";
    case tokens::TokenType::UNSAFE:
      return "#unsafe";
    case tokens::TokenType::SIMD:
      return "#simd";
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
      if (auto funcDecl =
              std::dynamic_pointer_cast<nodes::FunctionDeclNode>(node)) {
        visitFuncDecl(funcDecl.get());
      } else if (auto varDecl =
                     std::dynamic_pointer_cast<nodes::VarDeclNode>(node)) {
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

    if (auto genericFunc =
            std::dynamic_pointer_cast<nodes::GenericFunctionDeclNode>(node)) {
      visitFuncDecl(genericFunc.get());
    } else if (auto funcDecl =
                   std::dynamic_pointer_cast<nodes::FunctionDeclNode>(node)) {
      visitFuncDecl(funcDecl.get());
    } else if (auto varDecl =
                   std::dynamic_pointer_cast<nodes::VarDeclNode>(node)) {
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