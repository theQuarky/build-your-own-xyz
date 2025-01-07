#include "./utils.h"
#include "../parser/ast.h"
#include <iomanip>
#include <iostream>
#include <string>

// Helper function to get token type name
std::string getTokenTypeName(lexer::TokenType type) {
  switch (type) {
  // Keywords
  case lexer::TokenType::LET:
    return "LET";
  case lexer::TokenType::CONST:
    return "CONST";
  case lexer::TokenType::FUNCTION:
    return "FUNCTION";
  case lexer::TokenType::RETURN:
    return "RETURN";
  case lexer::TokenType::IF:
    return "IF";
  case lexer::TokenType::ELSE:
    return "ELSE";
  case lexer::TokenType::WHILE:
    return "WHILE";
  case lexer::TokenType::FOR:
    return "FOR";
  case lexer::TokenType::DO:
    return "DO";
  case lexer::TokenType::BREAK:
    return "BREAK";
  case lexer::TokenType::CONTINUE:
    return "CONTINUE";

  // Types
  case lexer::TokenType::TYPE_INT:
    return "TYPE_INT";
  case lexer::TokenType::TYPE_FLOAT:
    return "TYPE_FLOAT";
  case lexer::TokenType::TYPE_STRING:
    return "TYPE_STRING";
  case lexer::TokenType::TYPE_BOOLEAN:
    return "TYPE_BOOLEAN";

  // Brackets and braces
  case lexer::TokenType::LEFT_PAREN:
    return "LEFT_PAREN";
  case lexer::TokenType::RIGHT_PAREN:
    return "RIGHT_PAREN";
  case lexer::TokenType::LEFT_BRACE:
    return "LEFT_BRACE";
  case lexer::TokenType::RIGHT_BRACE:
    return "RIGHT_BRACE";
  case lexer::TokenType::LEFT_BRACKET:
    return "LEFT_BRACKET";
  case lexer::TokenType::RIGHT_BRACKET:
    return "RIGHT_BRACKET";

  // Literals
  case lexer::TokenType::NUMBER_LITERAL:
    return "NUMBER";
  case lexer::TokenType::STRING_LITERAL:
    return "STRING";
  case lexer::TokenType::BOOLEAN_LITERAL:
    return "BOOLEAN";
  case lexer::TokenType::NULL_LITERAL:
    return "NULL";
  case lexer::TokenType::UNDEFINED_LITERAL:
    return "UNDEFINED";

  // Operators
  case lexer::TokenType::PLUS:
    return "PLUS";
  case lexer::TokenType::MINUS:
    return "MINUS";
  case lexer::TokenType::MULTIPLY:
    return "MULTIPLY";
  case lexer::TokenType::DIVIDE:
    return "DIVIDE";
  case lexer::TokenType::MODULO:
    return "MODULO";
  case lexer::TokenType::POWER:
    return "POWER";

  // Bitwise Operators
  case lexer::TokenType::BITWISE_AND:
    return "BITWISE_AND";
  case lexer::TokenType::BITWISE_OR:
    return "BITWISE_OR";
  case lexer::TokenType::BITWISE_XOR:
    return "BITWISE_XOR";
  case lexer::TokenType::BITWISE_NOT:
    return "BITWISE_NOT";
  case lexer::TokenType::LEFT_SHIFT:
    return "LEFT_SHIFT";
  case lexer::TokenType::RIGHT_SHIFT:
    return "RIGHT_SHIFT";

  // Logical Operators
  case lexer::TokenType::AND:
    return "AND";
  case lexer::TokenType::OR:
    return "OR";
  case lexer::TokenType::NOT:
    return "NOT";

  // Comparison Operators
  case lexer::TokenType::EQUALS:
    return "EQUALS";
  case lexer::TokenType::NOT_EQUALS:
    return "NOT_EQUALS";
  case lexer::TokenType::LESS_THAN:
    return "LESS_THAN";
  case lexer::TokenType::GREATER_THAN:
    return "GREATER_THAN";
  case lexer::TokenType::LESS_EQUAL:
    return "LESS_EQUAL";
  case lexer::TokenType::GREATER_EQUAL:
    return "GREATER_EQUAL";

  // Assignment Operators
  case lexer::TokenType::ASSIGN:
    return "ASSIGN";
  case lexer::TokenType::PLUS_ASSIGN:
    return "PLUS_ASSIGN";
  case lexer::TokenType::MINUS_ASSIGN:
    return "MINUS_ASSIGN";
  case lexer::TokenType::MULTIPLY_ASSIGN:
    return "MULTIPLY_ASSIGN";
  case lexer::TokenType::DIVIDE_ASSIGN:
    return "DIVIDE_ASSIGN";
  case lexer::TokenType::MODULO_ASSIGN:
    return "MODULO_ASSIGN";
  case lexer::TokenType::AND_ASSIGN:
    return "AND_ASSIGN";
  case lexer::TokenType::OR_ASSIGN:
    return "OR_ASSIGN";
  case lexer::TokenType::XOR_ASSIGN:
    return "XOR_ASSIGN";
  case lexer::TokenType::LEFT_SHIFT_ASSIGN:
    return "LEFT_SHIFT_ASSIGN";
  case lexer::TokenType::RIGHT_SHIFT_ASSIGN:
    return "RIGHT_SHIFT_ASSIGN";

  // Increment/Decrement
  case lexer::TokenType::INCREMENT:
    return "INCREMENT";
  case lexer::TokenType::DECREMENT:
    return "DECREMENT";

  // Delimiters
  case lexer::TokenType::COMMA:
    return "COMMA";
  case lexer::TokenType::DOT:
    return "DOT";
  case lexer::TokenType::COLON:
    return "COLON";
  case lexer::TokenType::SEMICOLON:
    return "SEMICOLON";

  // Special tokens
  case lexer::TokenType::IDENTIFIER:
    return "IDENTIFIER";
  case lexer::TokenType::ERROR_TOKEN:
    return "ERROR";
  case lexer::TokenType::END_OF_FILE:
    return "EOF";

  default:
    return "UNKNOWN";
  }
}

// Helper function to print tokens in a formatted way
void printTokens(const std::vector<lexer::Token> &tokens) {
  std::cout << std::setfill('-') << std::setw(80) << "-" << std::endl;
  std::cout << std::setfill(' ');
  std::cout << std::left << std::setw(20) << "Token Type" << std::setw(20)
            << "Lexeme" << std::setw(10) << "Line" << std::setw(10) << "Column"
            << "Error Message" << std::endl;
  std::cout << std::setfill('-') << std::setw(80) << "-" << std::endl;
  std::cout << std::setfill(' ');

  for (const auto &token : tokens) {
    std::cout << std::left << std::setw(20) << getTokenTypeName(token.type)
              << std::setw(20) << token.lexeme << std::setw(10) << token.line
              << std::setw(10) << token.column;
    if (token.type == lexer::TokenType::ERROR_TOKEN) {
      std::cout << token.errorMessage;
    }
    std::cout << std::endl;
  }
  std::cout << std::setfill('-') << std::setw(80) << "-" << std::endl;
}

void printAST(const std::vector<ast::StmtPtr> &statements, int indent) {
  auto printIndent = [](int level) {
    std::cout << std::string(level * 2, ' ');
  };

  std::function<void(const ast::Node *, int)> printNode =
      [&](const ast::Node *node, int level) {
        if (!node)
          return;
        printIndent(level);

        if (auto lit = dynamic_cast<const ast::Literal *>(node)) {
          std::cout << "Literal(" << lit->value << ")\n";
        } else if (auto var = dynamic_cast<const ast::Variable *>(node)) {
          std::cout << "Variable('" << var->name
                    << "')\n"; // Now correctly prints the variable name
        } else if (auto bin = dynamic_cast<const ast::BinaryOp *>(node)) {
          std::cout << "BinaryOp('" << bin->token.lexeme << "')\n";
          printIndent(level + 1);
          std::cout << "Left:\n";
          printNode(bin->left.get(), level + 2);
          printIndent(level + 1);
          std::cout << "Right:\n";
          printNode(bin->right.get(), level + 2);
        } else if (auto un = dynamic_cast<const ast::UnaryOp *>(node)) {
          std::cout << "UnaryOp('" << un->token.lexeme << "')\n";
          printIndent(level + 1);
          std::cout << "Operand:\n";
          printNode(un->operand.get(), level + 2);
        } else if (auto call = dynamic_cast<const ast::Call *>(node)) {
          std::cout << "Call\n";
          printIndent(level + 1);
          std::cout << "Callee:\n";
          printNode(call->callee.get(), level + 2);

          if (!call->arguments.empty()) {
            printIndent(level + 1);
            std::cout << "Arguments:\n";
            for (const auto &arg : call->arguments) {
              printNode(arg.get(), level + 2);
            }
          }
        } else if (auto varDecl =
                       dynamic_cast<const ast::VarDeclaration *>(node)) {
          std::cout << "VarDeclaration '" << varDecl->name << "'\n";
          printIndent(level + 1);
          std::cout << "Kind: " << (varDecl->isConst ? "const" : "let") << "\n";
          if (varDecl->type) {
            printIndent(level + 1);
            std::cout << "Type: " << varDecl->type->getName() << "\n";
          }
          if (varDecl->initializer) {
            printIndent(level + 1);
            std::cout << "Initializer:\n";
            printNode(varDecl->initializer.get(), level + 2);
          }
        } else if (auto funDecl =
                       dynamic_cast<const ast::FunctionDeclaration *>(node)) {
          std::cout << "Function '" << funDecl->name << "'\n";

          if (!funDecl->parameters.empty()) {
            printIndent(level + 1);
            std::cout << "Parameters:\n";
            for (const auto &param : funDecl->parameters) {
              printIndent(level + 2);
              std::cout << param.first << ": " << param.second->getName()
                        << "\n";
            }
          }

          if (funDecl->returnType) {
            printIndent(level + 1);
            std::cout << "ReturnType: " << funDecl->returnType->getName()
                      << "\n";
          }

          printIndent(level + 1);
          std::cout << "Body:\n";
          for (const auto &stmt : funDecl->body) {
            printNode(stmt.get(), level + 2);
          }
        } else if (auto ret = dynamic_cast<const ast::Return *>(node)) {
          std::cout << "Return\n";
          if (ret->value) {
            printNode(ret->value.get(), level + 1);
          }
        } else if (auto ifStmt = dynamic_cast<const ast::If *>(node)) {
          std::cout << "If\n";
          printIndent(level + 1);
          std::cout << "Condition:\n";
          printNode(ifStmt->condition.get(), level + 2);

          printIndent(level + 1);
          std::cout << "Then:\n";
          printNode(ifStmt->thenBranch.get(), level + 2);

          if (ifStmt->elseBranch) {
            printIndent(level + 1);
            std::cout << "Else:\n";
            printNode(ifStmt->elseBranch.get(), level + 2);
          }
        } else if (auto block = dynamic_cast<const ast::Block *>(node)) {
          std::cout << "Block\n";
          for (const auto &stmt : block->statements) {
            printNode(stmt.get(), level + 1);
          }
        } else if (auto expr =
                       dynamic_cast<const ast::ExpressionStmt *>(node)) {
          std::cout << "ExpressionStatement\n";
          printNode(expr->expression.get(), level + 1);
        } else if (auto compound =
                       dynamic_cast<const ast::CompoundAssignment *>(node)) {
          std::cout << "CompoundAssignment('" << compound->op << "')\n";
          printIndent(level + 1);
          std::cout << "Target:\n";
          printNode(compound->target.get(), level + 2);
          printIndent(level + 1);
          std::cout << "Value:\n";
          printNode(compound->value.get(), level + 2);
        } else if (auto incDec =
                       dynamic_cast<const ast::IncrementDecrement *>(node)) {
          std::cout << (incDec->isIncrement ? "Increment" : "Decrement") << "("
                    << (incDec->isPrefix ? "prefix" : "postfix") << ")\n";
          printIndent(level + 1);
          std::cout << "Operand:\n";
          printNode(incDec->operand.get(), level + 2);
        }
      };

  for (const auto &stmt : statements) {
    printNode(stmt.get(), indent);
  }
}