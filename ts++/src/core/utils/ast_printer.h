#pragma once
#include "parser/ast.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/nodes/statement_nodes.h"
#include "parser/nodes/type_nodes.h"
#include <iostream>
#include <ostream>
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

  //---------------------------------------------------------------------------
  // 1) ClassDeclNode visitor
  //---------------------------------------------------------------------------
  void visitClassDecl(const nodes::ClassDeclNode *node) {
    indent();
    std::cout << BLUE << "ClassDecl" << RESET << " "
              << getLocationString(node->getLocation()) << "\n";

    indentLevel_++;

    // Print class name
    indent();
    std::cout << "Name: '" << node->getName() << "'\n";

    // Print class-level attributes (if your ClassDeclNode stores them)
    // e.g., #abstract, #aligned(16)
    const auto &classModifiers = node->getClassModifiers();
    if (!classModifiers.empty()) {
      indent();
      std::cout << "Class Modifiers:\n";
      indentLevel_++;
      for (auto mod : classModifiers) {
        indent();
        std::cout << tokenTypeToString(mod) << "\n";
      }
      indentLevel_--;
    }

    // Base class if any
    if (node->getBaseClass()) {
      indent();
      std::cout << "Base Class:\n";
      indentLevel_++;
      visitType(node->getBaseClass().get());
      indentLevel_--;
    }

    // Implemented interfaces
    const auto &interfaces = node->getInterfaces();
    if (!interfaces.empty()) {
      indent();
      std::cout << "Interfaces:\n";
      indentLevel_++;
      for (const auto &iface : interfaces) {
        visitType(iface.get());
      }
      indentLevel_--;
    }

    // Class members
    const auto &members = node->getMembers();
    if (!members.empty()) {
      indent();
      std::cout << "Members:\n";
      indentLevel_++;
      for (const auto &member : members) {
        print(member); // dispatch to the correct visitor method
      }
      indentLevel_--;
    }

    indentLevel_--;
  }

  //---------------------------------------------------------------------------
  // 2) MethodDeclNode visitor
  //---------------------------------------------------------------------------
  void visitMethodDecl(const nodes::MethodDeclNode *node) {
    indent();
    std::cout << BLUE << "MethodDecl" << RESET << " "
              << getLocationString(node->getLocation()) << "\n";

    indentLevel_++;

    // Access modifier
    indent();
    std::cout << "Access: " << tokenTypeToString(node->getAccessModifier())
              << "\n";

    // Method name
    indent();
    std::cout << "Name: '" << node->getName() << "'\n";

    // Parameters
    const auto &parameters = node->getParameters();
    if (!parameters.empty()) {
      indent();
      std::cout << "Parameters:\n";
      indentLevel_++;
      for (const auto &param : parameters) {
        visitParameter(param.get());
      }
      indentLevel_--;
    }

    // Return type
    if (node->getReturnType()) {
      indent();
      std::cout << "Return Type:\n";
      indentLevel_++;
      visitType(node->getReturnType().get());
      indentLevel_--;
    }

    // Throws types
    if (!node->getThrowsTypes().empty()) {
      indent();
      std::cout << "Throws:\n";
      indentLevel_++;
      for (const auto &throwT : node->getThrowsTypes()) {
        visitType(throwT.get());
      }
      indentLevel_--;
    }

    // Method modifiers (like #inline, #virtual)
    if (!node->getModifiers().empty()) {
      indent();
      std::cout << "Method Modifiers:\n";
      indentLevel_++;
      for (auto mod : node->getModifiers()) {
        indent();
        std::cout << modifierToString(mod) << "\n";
      }
      indentLevel_--;
    }

    // Body
    if (node->getBody()) {
      indent();
      std::cout << "Body:\n";
      indentLevel_++;
      visitBlock(node->getBody().get());
      indentLevel_--;
    }

    indentLevel_--;
  }

  //---------------------------------------------------------------------------
  // 3) ConstructorDeclNode visitor
  //---------------------------------------------------------------------------
  void visitConstructorDecl(const nodes::ConstructorDeclNode *node) {
    indent();
    std::cout << BLUE << "ConstructorDecl" << RESET << " "
              << getLocationString(node->getLocation()) << "\n";

    indentLevel_++;

    // Access
    indent();
    std::cout << "Access: " << tokenTypeToString(node->getAccessModifier())
              << "\n";

    // Parameters
    const auto &params = node->getParameters();
    if (!params.empty()) {
      indent();
      std::cout << "Parameters:\n";
      indentLevel_++;
      for (const auto &param : params) {
        visitParameter(param.get());
      }
      indentLevel_--;
    }

    // Body
    if (node->getBody()) {
      indent();
      std::cout << "Body:\n";
      indentLevel_++;
      visitBlock(node->getBody().get());
      indentLevel_--;
    }

    indentLevel_--;
  }

  //---------------------------------------------------------------------------
  // 4) FieldDeclNode visitor
  //---------------------------------------------------------------------------
  void visitFieldDecl(const nodes::FieldDeclNode *node) {
    indent();
    std::cout << GREEN << "FieldDecl" << RESET << " "
              << getLocationString(node->getLocation()) << "\n";

    indentLevel_++;

    // Access
    indent();
    std::cout << "Access: " << tokenTypeToString(node->getAccessModifier())
              << "\n";

    // isConst
    if (node->isConst()) {
      indent();
      std::cout << "Const: true\n";
    }

    // Name
    indent();
    std::cout << "Name: '" << node->getName() << "'\n";

    // Type
    if (node->getType()) {
      indent();
      std::cout << "Type:\n";
      indentLevel_++;
      visitType(node->getType().get());
      indentLevel_--;
    }

    // Initializer
    if (node->getInitializer()) {
      indent();
      std::cout << "Initializer:\n";
      indentLevel_++;
      visitExpr(node->getInitializer().get());
      indentLevel_--;
    }

    indentLevel_--;
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

  void visitStmt(const nodes::StatementNode *stmt) {
    if (!stmt) {
      indent();
      std::cout << RED << "null-statement" << RESET << "\n";
      return;
    }

    // Handle all statement types
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
    } else if (auto whileStmt =
                   dynamic_cast<const nodes::WhileStmtNode *>(stmt)) {
      visitWhileStmt(whileStmt);
    } else if (auto doWhileStmt =
                   dynamic_cast<const nodes::DoWhileStmtNode *>(stmt)) {
      visitDoWhileStmt(doWhileStmt);
    } else if (auto blockStmt = dynamic_cast<const nodes::BlockNode *>(stmt)) {
      visitBlock(blockStmt);
    } else if (auto breakStmt =
                   dynamic_cast<const nodes::BreakStmtNode *>(stmt)) {
      visitBreakStmt(breakStmt);
    } else if (auto continueStmt =
                   dynamic_cast<const nodes::ContinueStmtNode *>(stmt)) {
      visitContinueStmt(continueStmt);
    } else {
      indent();
      std::cout << RED << "Unknown statement type at "
                << stmt->getLocation().getLine() << ":"
                << stmt->getLocation().getColumn() << RESET << "\n";
    }
  }

  // Add these new visit methods:
  void visitWhileStmt(const nodes::WhileStmtNode *node) {
    indent();
    std::cout << "While " << getLocationString(node->getLocation()) << "\n";

    indentLevel_++;

    indent();
    std::cout << "Condition:\n";
    indentLevel_++;
    visitExpr(node->getCondition().get());
    indentLevel_--;

    indent();
    std::cout << "Body:\n";
    indentLevel_++;
    visitStmt(node->getBody().get());
    indentLevel_--;

    indentLevel_--;
  }

  void visitDoWhileStmt(const nodes::DoWhileStmtNode *node) {
    indent();
    std::cout << "DoWhile " << getLocationString(node->getLocation()) << "\n";

    indentLevel_++;

    indent();
    std::cout << "Body:\n";
    indentLevel_++;
    visitStmt(node->getBody().get());
    indentLevel_--;

    indent();
    std::cout << "Condition:\n";
    indentLevel_++;
    visitExpr(node->getCondition().get());
    indentLevel_--;

    indentLevel_--;
  }

  void visitBreakStmt(const nodes::BreakStmtNode *node) {
    indent();
    std::cout << "Break";
    if (!node->getLabel().empty()) {
      std::cout << " " << node->getLabel();
    }
    std::cout << " " << getLocationString(node->getLocation()) << "\n";
  }

  void visitContinueStmt(const nodes::ContinueStmtNode *node) {
    indent();
    std::cout << "Continue";
    if (!node->getLabel().empty()) {
      std::cout << " " << node->getLabel();
    }
    std::cout << " " << getLocationString(node->getLocation()) << "\n";
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
    case tokens::TokenType::PUBLIC:
      return "public";
    case tokens::TokenType::PRIVATE:
      return "private";
    case tokens::TokenType::PROTECTED:
      return "protected";
    case tokens::TokenType::INLINE:
      return "#inline";
    case tokens::TokenType::VIRTUAL:
      return "#virtual";
    case tokens::TokenType::UNSAFE:
      return "#unsafe";
    case tokens::TokenType::SIMD:
      return "#simd";
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

      // 1) Check if it’s a ClassDeclNode
      if (auto classDecl =
              std::dynamic_pointer_cast<nodes::ClassDeclNode>(node)) {
        visitClassDecl(classDecl.get());
      }
      // 2) MethodDeclNode
      else if (auto methodDecl =
                   std::dynamic_pointer_cast<nodes::MethodDeclNode>(node)) {
        visitMethodDecl(methodDecl.get());
      }
      // 3) ConstructorDeclNode
      else if (auto ctorDecl =
                   std::dynamic_pointer_cast<nodes::ConstructorDeclNode>(
                       node)) {
        visitConstructorDecl(ctorDecl.get());
      }
      // 4) FieldDeclNode
      else if (auto fieldDecl =
                   std::dynamic_pointer_cast<nodes::FieldDeclNode>(node)) {
        visitFieldDecl(fieldDecl.get());
      } else if (auto funcDecl =
                     std::dynamic_pointer_cast<nodes::FunctionDeclNode>(node)) {
        std::cout << "calling function declaration" << std::endl;
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
    // 1) Check if it’s a ClassDeclNode
    if (auto classDecl =
            std::dynamic_pointer_cast<nodes::ClassDeclNode>(node)) {
      visitClassDecl(classDecl.get());
    }
    // 2) MethodDeclNode
    else if (auto methodDecl =
                 std::dynamic_pointer_cast<nodes::MethodDeclNode>(node)) {
      visitMethodDecl(methodDecl.get());
    }
    // 3) ConstructorDeclNode
    else if (auto ctorDecl =
                 std::dynamic_pointer_cast<nodes::ConstructorDeclNode>(node)) {
      visitConstructorDecl(ctorDecl.get());
    }
    // 4) FieldDeclNode
    else if (auto fieldDecl =
                 std::dynamic_pointer_cast<nodes::FieldDeclNode>(node)) {
      visitFieldDecl(fieldDecl.get());
    } else if (auto genericFunc =
                   std::dynamic_pointer_cast<nodes::GenericFunctionDeclNode>(
                       node)) {
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