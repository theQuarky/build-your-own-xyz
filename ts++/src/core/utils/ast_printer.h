/**
 * ASTPrinter.h
 * Purpose: Pretty-prints Abstract Syntax Tree (AST) nodes with color coding and
 *          proper indentation for debugging and visualization.
 */

#pragma once
#include "parser/ast.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/nodes/statement_nodes.h"
#include "parser/nodes/type_nodes.h"
#include "tokens/token_type.h"
#include <memory>
#include <string_view>
#include <unordered_map>

namespace core {

class ASTPrinter {
public:
  // Main public interface
  void print(const parser::AST &ast);
  void print(const nodes::NodePtr &node);

private:
  // ANSI color codes for syntax highlighting
  static constexpr std::string_view RESET = "\033[0m";
  static constexpr std::string_view RED = "\033[31m";    // Errors & nullptrs
  static constexpr std::string_view GREEN = "\033[32m";  // Declarations
  static constexpr std::string_view YELLOW = "\033[33m"; // Expressions
  static constexpr std::string_view BLUE = "\033[34m";   // Keywords & types

  // Current indentation state
  int indentLevel_ = 0;

  // RAII guard for indentation management
  class IndentGuard {
  public:
    explicit IndentGuard(ASTPrinter &printer) : printer_(printer) {
      printer_.indentLevel_++;
    }
    ~IndentGuard() { printer_.indentLevel_--; }

  private:
    ASTPrinter &printer_;
  };

  // Helper methods
  void indent() const { std::cout << std::string(indentLevel_ * 2, ' '); }

  std::string getLocationString(const core::SourceLocation &loc) const {
    return "(" + std::to_string(loc.getLine()) + ":" +
           std::to_string(loc.getColumn()) + ")";
  }

  // Template for printing sections with automatic indentation
  template <typename Func>
  void printSection(std::string_view title, const Func &printFunc) {
    indent();
    std::cout << title << "\n";
    IndentGuard guard(*this);
    printFunc();
  }

  // Node type handlers
  void handleDeclaration(const nodes::NodePtr &node) {
    if (!node) {
      indent();
      std::cout << RED << "null-declaration" << RESET << "\n";
      return;
    }

    if (auto classDecl =
            std::dynamic_pointer_cast<nodes::ClassDeclNode>(node)) {
      visitClassDecl(classDecl.get());
    } else if (auto methodDecl =
                   std::dynamic_pointer_cast<nodes::MethodDeclNode>(node)) {
      visitMethodDecl(methodDecl.get());
    } else if (auto funcDecl =
                   std::dynamic_pointer_cast<nodes::FunctionDeclNode>(node)) {
      visitFunctionDecl(funcDecl.get());
    } else if (auto varDecl =
                   std::dynamic_pointer_cast<nodes::VarDeclNode>(node)) {
      visitVariableDecl(varDecl.get());
    } else if (auto stmt =
                   std::dynamic_pointer_cast<nodes::StatementNode>(node)) {
      visitStatement(stmt.get());
    } else {
      indent();
      std::cout << RED << "Unknown declaration type: " << typeid(*node).name()
                << RESET << "\n";
    }
  }

  // Core visitors
  void visitClassDecl(const nodes::ClassDeclNode *node) {
    if (!node)
      return;

    printSection(BLUE + std::string("ClassDecl") + RESET + " " +
                     getLocationString(node->getLocation()),
                 [&] {
                   // Print name
                   indent();
                   std::cout << "Name: '" << node->getName() << "'\n";

                   // Print modifiers
                   if (!node->getClassModifiers().empty()) {
                     printSection("Modifiers:", [&] {
                       for (auto mod : node->getClassModifiers()) {
                         indent();
                         std::cout << modifierToString(mod) << "\n";
                       }
                     });
                   }

                   // Print base class
                   if (node->getBaseClass()) {
                     printSection("Base Class:",
                                  [&] { visitType(node->getBaseClass()); });
                   }

                   // Print interfaces
                   if (!node->getInterfaces().empty()) {
                     printSection("Interfaces:", [&] {
                       for (const auto &iface : node->getInterfaces()) {
                         visitType(iface);
                       }
                     });
                   }

                   // Print members
                   if (!node->getMembers().empty()) {
                     printSection("Members:", [&] {
                       for (const auto &member : node->getMembers()) {
                         handleDeclaration(member);
                       }
                     });
                   }
                 });
  }

  void visitMethodDecl(const nodes::MethodDeclNode *node) {
    if (!node)
      return;

    printSection(
        BLUE + std::string("MethodDecl") + RESET + " " +
            getLocationString(node->getLocation()),
        [&] {
          // Access modifier
          indent();
          std::cout << "Access: "
                    << tokenTypeToString(node->getAccessModifier()) << "\n";

          // Name
          indent();
          std::cout << "Name: '" << node->getName() << "'\n";

          // Parameters
          if (!node->getParameters().empty()) {
            printSection("Parameters:", [&] {
              for (const auto &param : node->getParameters()) {
                visitParameter(param.get());
              }
            });
          }

          // Return type
          if (node->getReturnType()) {
            printSection("Return Type:",
                         [&] { visitType(node->getReturnType()); });
          }

          // Body
          if (node->getBody()) {
            printSection("Body:", [&] { visitBlock(node->getBody().get()); });
          }
        });
  }

  void visitFunctionDecl(const nodes::FunctionDeclNode *node) {
    if (!node)
      return;

    printSection(
        BLUE + std::string("FunctionDecl") + RESET + " " +
            getLocationString(node->getLocation()),
        [&] {
          // Name
          indent();
          std::cout << "Name: '" << node->getName() << "'\n";

          // Print generic info if available
          if (auto genericFunc =
                  dynamic_cast<const nodes::GenericFunctionDeclNode *>(node)) {
            if (!genericFunc->getGenericParams().empty()) {
              printSection("Generic Parameters:", [&] {
                for (const auto &param : genericFunc->getGenericParams()) {
                  indent();
                  std::cout << param->toString() << "\n";
                }
              });
            }
          }

          // Parameters
          if (!node->getParameters().empty()) {
            printSection("Parameters:", [&] {
              for (const auto &param : node->getParameters()) {
                visitParameter(param.get());
              }
            });
          }

          // Return type
          if (node->getReturnType()) {
            printSection("Return Type:",
                         [&] { visitType(node->getReturnType()); });
          }

          // Body
          if (node->getBody()) {
            printSection("Body:", [&] { visitBlock(node->getBody().get()); });
          }
        });
  }

  void visitVariableDecl(const nodes::VarDeclNode *node) {
    if (!node)
      return;

    printSection(GREEN + std::string("VarDecl") + RESET + " " +
                     getLocationString(node->getLocation()),
                 [&] {
                   indent();
                   std::cout << "Name: '" << node->getName() << "'\n";

                   if (node->isConst()) {
                     indent();
                     std::cout << "Const: true\n";
                   }

                   if (node->getType()) {
                     printSection("Type:", [&] { visitType(node->getType()); });
                   }

                   if (node->getInitializer()) {
                     printSection("Initializer:", [&] {
                       visitExpression(node->getInitializer().get());
                     });
                   }
                 });
  }

  void visitParameter(const nodes::ParameterNode *node) {
    if (!node)
      return;

    printSection(YELLOW + std::string("Parameter") + RESET + " '" +
                     node->getName() + "' " +
                     getLocationString(node->getLocation()),
                 [&] {
                   if (node->getType()) {
                     printSection("Type:", [&] { visitType(node->getType()); });
                   }

                   if (node->isRef()) {
                     indent();
                     std::cout << "Ref: true\n";
                   }

                   if (node->isConst()) {
                     indent();
                     std::cout << "Const: true\n";
                   }

                   if (node->getDefaultValue()) {
                     printSection("Default Value:", [&] {
                       visitExpression(node->getDefaultValue().get());
                     });
                   }
                 });
  }

  void visitBlock(const nodes::BlockNode *node) {
    if (!node)
      return;

    indent();
    std::cout << "Block " << getLocationString(node->getLocation()) << "\n";

    IndentGuard guard(*this);
    for (const auto &stmt : node->getStatements()) {
      visitStatement(stmt.get());
    }
  }

  void visitStatement(const nodes::StatementNode *stmt) {
    if (!stmt) {
      indent();
      std::cout << RED << "null-statement" << RESET << "\n";
      return;
    }

    if (auto exprStmt = dynamic_cast<const nodes::ExpressionStmtNode *>(stmt)) {
      visitExpressionStmt(exprStmt);
    } else if (auto returnStmt =
                   dynamic_cast<const nodes::ReturnStmtNode *>(stmt)) {
      visitReturnStmt(returnStmt);
    } else if (auto ifStmt = dynamic_cast<const nodes::IfStmtNode *>(stmt)) {
      visitIfStmt(ifStmt);
    } else if (auto blockStmt = dynamic_cast<const nodes::BlockNode *>(stmt)) {
      visitBlock(blockStmt);
    } else {
      indent();
      std::cout << RED << "Unknown statement type: " << typeid(*stmt).name()
                << RESET << "\n";
    }
  }

  void visitExpressionStmt(const nodes::ExpressionStmtNode *node) {
    if (!node)
      return;

    printSection("ExpressionStatement " +
                     getLocationString(node->getLocation()),
                 [&] { visitExpression(node->getExpression().get()); });
  }

  void visitExpression(const nodes::ExpressionNode *expr) {
    if (!expr) {
      indent();
      std::cout << RED << "null-expression" << RESET << "\n";
      return;
    }

    if (auto binary = dynamic_cast<const nodes::BinaryExpressionNode *>(expr)) {
      visitBinaryExpr(binary);
    } else if (auto unary =
                   dynamic_cast<const nodes::UnaryExpressionNode *>(expr)) {
      visitUnaryExpr(unary);
    } else if (auto literal =
                   dynamic_cast<const nodes::LiteralExpressionNode *>(expr)) {
      visitLiteralExpr(literal);
    } else if (auto ident =
                   dynamic_cast<const nodes::IdentifierExpressionNode *>(
                       expr)) {
      visitIdentifierExpr(ident);
    } else {
      indent();
      std::cout << "Expression: "
                << tokenTypeToString(expr->getExpressionType()) << " "
                << getLocationString(expr->getLocation()) << "\n";
    }
  }

  void visitBinaryExpr(const nodes::BinaryExpressionNode *expr) {
    if (!expr)
      return;

    printSection(
        "BinaryExpression: " + tokenTypeToString(expr->getExpressionType()) +
            " " + getLocationString(expr->getLocation()),
        [&] {
          printSection("Left:",
                       [&] { visitExpression(expr->getLeft().get()); });
          printSection("Right:",
                       [&] { visitExpression(expr->getRight().get()); });
        });
  }

  void visitUnaryExpr(const nodes::UnaryExpressionNode *expr) {
    if (!expr)
      return;

    printSection(
        "UnaryExpression " +
            std::string(expr->isPrefix() ? "(prefix) " : "(postfix) ") +
            tokenTypeToString(expr->getExpressionType()) + " " +
            getLocationString(expr->getLocation()),
        [&] {
          printSection("Operand:",
                       [&] { visitExpression(expr->getOperand().get()); });
        });
  }

  void visitLiteralExpr(const nodes::LiteralExpressionNode *expr) {
    if (!expr)
      return;

    indent();
    std::cout << "Literal: '" << expr->getValue() << "' "
              << getLocationString(expr->getLocation()) << "\n";
  }

  void visitIdentifierExpr(const nodes::IdentifierExpressionNode *expr) {
    if (!expr)
      return;

    indent();
    std::cout << "Identifier: '" << expr->getName() << "' "
              << getLocationString(expr->getLocation()) << "\n";
  }

  void visitType(const nodes::TypePtr &type) {
    if (!type) {
      indent();
      std::cout << RED << "null-type" << RESET << "\n";
      return;
    }

    indent();
    std::cout << type->toString() << "\n";

    IndentGuard guard(*this);
    if (auto arrayType =
            dynamic_cast<const nodes::ArrayTypeNode *>(type.get())) {
      if (arrayType->getElementType()) {
        printSection("ElementType:",
                     [&] { visitType(arrayType->getElementType()); });
      }
    }
  }

  // Token conversion utilities
  static std::string tokenTypeToString(tokens::TokenType type) {
    static const std::unordered_map<tokens::TokenType, std::string>
        tokenStrings = {{tokens::TokenType::PLUS, "+"},
                        {tokens::TokenType::MINUS, "-"},
                        {tokens::TokenType::STAR, "*"},
                        {tokens::TokenType::SLASH, "/"},
                        {tokens::TokenType::EQUALS, "="},
                        {tokens::TokenType::PUBLIC, "public"},
                        {tokens::TokenType::PRIVATE, "private"},
                        {tokens::TokenType::PROTECTED, "protected"},
                        {tokens::TokenType::VOID, "void"},
                        {tokens::TokenType::INT, "int"},
                        {tokens::TokenType::FLOAT, "float"},
                        {tokens::TokenType::BOOLEAN, "bool"},
                        {tokens::TokenType::STRING, "string"},
                        {tokens::TokenType::PLUS_EQUALS, "+="},
                        {tokens::TokenType::MINUS_EQUALS, "-="},
                        {tokens::TokenType::STAR_EQUALS, "*="},
                        {tokens::TokenType::SLASH_EQUALS, "/="},
                        {tokens::TokenType::PLUS_PLUS, "++"},
                        {tokens::TokenType::MINUS_MINUS, "--"},
                        {tokens::TokenType::PERCENT, "%"},
                        {tokens::TokenType::GREATER, ">"},
                        {tokens::TokenType::LESS, "<"},
                        {tokens::TokenType::GREATER_EQUALS, ">="},
                        {tokens::TokenType::LESS_EQUALS, "<="},
                        {tokens::TokenType::STACK, "#stack"},
                        {tokens::TokenType::HEAP, "#heap"},
                        {tokens::TokenType::STATIC, "#static"},
                        {tokens::TokenType::OF, "of"}};

    auto it = tokenStrings.find(type);
    if (it != tokenStrings.end()) {
      return it->second;
    }
    return std::to_string(static_cast<int>(type));
  }

  static std::string modifierToString(tokens::TokenType modifier) {
    static const std::unordered_map<tokens::TokenType, std::string>
        modifierStrings = {{tokens::TokenType::INLINE, "#inline"},
                           {tokens::TokenType::VIRTUAL, "#virtual"},
                           {tokens::TokenType::UNSAFE, "#unsafe"},
                           {tokens::TokenType::SIMD, "#simd"},
                           {tokens::TokenType::ALIGNED, "#aligned"},
                           {tokens::TokenType::PACKED, "#packed"},
                           {tokens::TokenType::ABSTRACT, "#abstract"}};

    auto it = modifierStrings.find(modifier);
    if (it != modifierStrings.end()) {
      return it->second;
    }
    return "unknown-modifier";
  }

  // Additional statement visitors
  void visitReturnStmt(const nodes::ReturnStmtNode *node) {
    if (!node)
      return;

    printSection("Return " + getLocationString(node->getLocation()), [&] {
      if (node->getValue()) {
        visitExpression(node->getValue().get());
      }
    });
  }

  void visitIfStmt(const nodes::IfStmtNode *node) {
    if (!node)
      return;

    printSection("If " + getLocationString(node->getLocation()), [&] {
      printSection("Condition:",
                   [&] { visitExpression(node->getCondition().get()); });

      printSection("Then:",
                   [&] { visitStatement(node->getThenBranch().get()); });

      if (node->getElseBranch()) {
        printSection("Else:",
                     [&] { visitStatement(node->getElseBranch().get()); });
      }
    });
  }

  void visitLoopStmt(const nodes::WhileStmtNode *node) {
    if (!node)
      return;

    printSection("While " + getLocationString(node->getLocation()), [&] {
      printSection("Condition:",
                   [&] { visitExpression(node->getCondition().get()); });

      printSection("Body:", [&] { visitStatement(node->getBody().get()); });
    });
  }

  void visitForStmt(const nodes::ForStmtNode *node) {
    if (!node)
      return;

    printSection("For " + getLocationString(node->getLocation()), [&] {
      printSection("Initializer:", [&] {
        if (node->getInitializer()) {
          visitStatement(node->getInitializer().get());
        } else {
          indent();
          std::cout << "<empty>\n";
        }
      });

      printSection("Condition:", [&] {
        if (node->getCondition()) {
          visitExpression(node->getCondition().get());
        } else {
          indent();
          std::cout << "<empty>\n";
        }
      });

      printSection("Increment:", [&] {
        if (node->getIncrement()) {
          visitExpression(node->getIncrement().get());
        } else {
          indent();
          std::cout << "<empty>\n";
        }
      });

      printSection("Body:", [&] { visitStatement(node->getBody().get()); });
    });
  }

  void visitForOfStmt(const nodes::ForOfStmtNode *node) {
    if (!node)
      return;

    printSection("ForOf " + getLocationString(node->getLocation()), [&] {
      indent();
      std::cout << (node->isConst() ? "const " : "let ")
                << node->getIdentifier() << "\n";

      printSection("Iterable:",
                   [&] { visitExpression(node->getIterable().get()); });

      printSection("Body:", [&] { visitStatement(node->getBody().get()); });
    });
  }

  void visitTryStmt(const nodes::TryStmtNode *node) {
    if (!node)
      return;

    printSection("Try " + getLocationString(node->getLocation()), [&] {
      printSection("Try Block:",
                   [&] { visitStatement(node->getTryBlock().get()); });

      if (!node->getCatchClauses().empty()) {
        printSection("Catch Clauses:", [&] {
          for (const auto &catchClause : node->getCatchClauses()) {
            printSection("Catch:", [&] {
              indent();
              std::cout << "Parameter: " << catchClause.parameter << "\n";
              if (catchClause.parameterType) {
                printSection("Type:",
                             [&] { visitType(catchClause.parameterType); });
              }
              if (catchClause.body) {
                printSection("Body:",
                             [&] { visitStatement(catchClause.body.get()); });
              }
            });
          }
        });
      }

      if (node->getFinallyBlock()) {
        printSection("Finally:",
                     [&] { visitStatement(node->getFinallyBlock().get()); });
      }
    });
  }
};

} // namespace core