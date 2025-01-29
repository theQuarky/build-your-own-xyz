#include "log_utils.h"
#include "tokens/token_type.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/nodes/type_nodes.h"
#include <iomanip>
#include <iostream>
#include <unordered_map>

namespace {
const char *RESET = "\033[0m";
const char *RED = "\033[31m";
const char *GREEN = "\033[32m";
const char *YELLOW = "\033[33m";
const char *BLUE = "\033[34m";
const char *MAGENTA = "\033[35m";
const char *CYAN = "\033[36m";

std::string getTokenCategory(tokens::TokenType type) {
  if (tokens::isDeclaration(type))
    return "Declaration";
  if (tokens::isAssignmentOperator(type))
    return "Assignment";
  if (tokens::isType(type))
    return "Type";
  if (tokens::isControlFlow(type))
    return "Control Flow";
  if (tokens::isClassRelated(type))
    return "Class";
  if (tokens::isOperator(type))
    return "Operator";
  if (tokens::isLiteral(type))
    return "Literal";
  if (tokens::isDelimiter(type))
    return "Delimiter";
  if (tokens::isSpecial(type))
    return "Special";
  if (tokens::isMemoryManagement(type))
    return "Memory";
  if (tokens::isAccessModifier(type))
    return "Access";
  if (tokens::isFunctionModifier(type))
    return "Function";
  if (tokens::isModernFeature(type))
    return "Modern";
  return "Unknown";
}

const std::unordered_map<tokens::TokenType, std::string> tokenTypeStrings = {
    // Declaration
    {tokens::TokenType::LET, "LET"},
    {tokens::TokenType::CONST, "CONST"},
    {tokens::TokenType::FUNCTION, "FUNCTION"},
    {tokens::TokenType::CLASS, "CLASS"},
    {tokens::TokenType::INTERFACE, "INTERFACE"},
    {tokens::TokenType::ENUM, "ENUM"},
    {tokens::TokenType::TYPEDEF, "TYPEDEF"},
    {tokens::TokenType::NAMESPACE, "NAMESPACE"},
    {tokens::TokenType::TEMPLATE, "TEMPLATE"},
    {tokens::TokenType::NEW, "NEW"},
    {tokens::TokenType::THROW, "THROW"},
    {tokens::TokenType::TYPEOF, "TYPEOF"},

    // Class related
    {tokens::TokenType::EXTENDS, "EXTENDS"},
    {tokens::TokenType::IMPLEMENTS, "IMPLEMENTS"},
    {tokens::TokenType::THROWS, "THROWS"},

    // Access Modifiers
    {tokens::TokenType::PUBLIC, "PUBLIC"},
    {tokens::TokenType::PRIVATE, "PRIVATE"},
    {tokens::TokenType::PROTECTED, "PROTECTED"},

    // Memory Management
    {tokens::TokenType::STACK, "STACK"},
    {tokens::TokenType::HEAP, "HEAP"},
    {tokens::TokenType::STATIC, "STATIC"},
    {tokens::TokenType::SHARED, "SHARED"},
    {tokens::TokenType::UNIQUE, "UNIQUE"},
    {tokens::TokenType::WEAK, "WEAK"},
    {tokens::TokenType::AT, "ADDRESS_OF"},

    // Types
    {tokens::TokenType::VOID, "VOID"},
    {tokens::TokenType::INT, "INT"},
    {tokens::TokenType::FLOAT, "FLOAT"},
    {tokens::TokenType::BOOLEAN, "BOOLEAN"},
    {tokens::TokenType::STRING, "STRING"},

    // Control Flow
    {tokens::TokenType::IF, "IF"},
    {tokens::TokenType::ELSE, "ELSE"},
    {tokens::TokenType::FOR, "FOR"},
    {tokens::TokenType::WHILE, "WHILE"},
    {tokens::TokenType::DO, "DO"},
    {tokens::TokenType::BREAK, "BREAK"},
    {tokens::TokenType::CONTINUE, "CONTINUE"},
    {tokens::TokenType::RETURN, "RETURN"},
    {tokens::TokenType::THROW, "THROW"},
    {tokens::TokenType::TRY, "TRY"},
    {tokens::TokenType::CATCH, "CATCH"},
    {tokens::TokenType::FINALLY, "FINALLY"},
    {tokens::TokenType::SWITCH, "SWITCH"},
    {tokens::TokenType::CASE, "CASE"},
    {tokens::TokenType::DEFAULT, "DEFAULT"},

    // Operators
    {tokens::TokenType::PLUS, "PLUS"},
    {tokens::TokenType::MINUS, "MINUS"},
    {tokens::TokenType::STAR, "STAR"},
    {tokens::TokenType::SLASH, "SLASH"},
    {tokens::TokenType::PERCENT, "PERCENT"},
    {tokens::TokenType::EQUALS_EQUALS, "EQUALS_EQUALS"},
    {tokens::TokenType::EXCLAIM_EQUALS, "EXCLAIM_EQUALS"},
    {tokens::TokenType::LESS, "LESS"},
    {tokens::TokenType::GREATER, "GREATER"},
    {tokens::TokenType::LESS_EQUALS, "LESS_EQUALS"},
    {tokens::TokenType::GREATER_EQUALS, "GREATER_EQUALS"},
    {tokens::TokenType::AMPERSAND, "AMPERSAND"},
    {tokens::TokenType::PIPE, "PIPE"},
    {tokens::TokenType::CARET, "CARET"},
    {tokens::TokenType::TILDE, "TILDE"},
    {tokens::TokenType::EXCLAIM, "EXCLAIM"},
    {tokens::TokenType::AMPERSAND_AMPERSAND, "AMPERSAND_AMPERSAND"},
    {tokens::TokenType::PIPE_PIPE, "PIPE_PIPE"},
    {tokens::TokenType::RIGHT_SHIFT, "RIGHT_SHIFT"},
    {tokens::TokenType::LEFT_SHIFT, "LEFT_SHIFT"},

    // Assignment Operators
    {tokens::TokenType::EQUALS, "EQUALS"},
    {tokens::TokenType::PLUS_EQUALS, "PLUS_EQUALS"},
    {tokens::TokenType::MINUS_EQUALS, "MINUS_EQUALS"},
    {tokens::TokenType::STAR_EQUALS, "STAR_EQUALS"},
    {tokens::TokenType::SLASH_EQUALS, "SLASH_EQUALS"},
    {tokens::TokenType::PERCENT_EQUALS, "PERCENT_EQUALS"},
    {tokens::TokenType::AMPERSAND_EQUALS, "AMPERSAND_EQUALS"},
    {tokens::TokenType::PIPE_EQUALS, "PIPE_EQUALS"},
    {tokens::TokenType::CARET_EQUALS, "CARET_EQUALS"},

    // Literals
    {tokens::TokenType::IDENTIFIER, "IDENTIFIER"},
    {tokens::TokenType::NUMBER, "NUMBER"},
    {tokens::TokenType::STRING_LITERAL, "STRING_LITERAL"},
    {tokens::TokenType::CHAR_LITERAL, "CHAR_LITERAL"},
    {tokens::TokenType::TRUE, "TRUE"},
    {tokens::TokenType::FALSE, "FALSE"},
    {tokens::TokenType::NULL_VALUE, "NULL"},
    {tokens::TokenType::UNDEFINED, "UNDEFINED"},
    {tokens::TokenType::THIS, "THIS"},

    // Delimiters
    {tokens::TokenType::LEFT_PAREN, "LEFT_PAREN"},
    {tokens::TokenType::RIGHT_PAREN, "RIGHT_PAREN"},
    {tokens::TokenType::LEFT_BRACE, "LEFT_BRACE"},
    {tokens::TokenType::RIGHT_BRACE, "RIGHT_BRACE"},
    {tokens::TokenType::LEFT_BRACKET, "LEFT_BRACKET"},
    {tokens::TokenType::RIGHT_BRACKET, "RIGHT_BRACKET"},
    {tokens::TokenType::SEMICOLON, "SEMICOLON"},
    {tokens::TokenType::COLON, "COLON"},
    {tokens::TokenType::DOT, "DOT"},
    {tokens::TokenType::COMMA, "COMMA"},
    {tokens::TokenType::ATTRIBUTE, "ATTRIBUTE"},

    // Special
    {tokens::TokenType::ERROR_TOKEN, "ERROR"},
    {tokens::TokenType::END_OF_FILE, "EOF"}};

} // anonymous namespace

void printToken(const tokens::Token &token) {
  std::cout << "Token{type=";
  auto it = tokenTypeStrings.find(token.getType());
  std::cout << (it != tokenTypeStrings.end() ? it->second : "UNKNOWN");

  std::cout << ", category=\"" << getTokenCategory(token.getType()) << "\""
            << ", lexeme=\"" << token.getLexeme() << "\""
            << ", line=" << token.getLocation().getLine()
            << ", column=" << token.getLocation().getColumn();

  if (const auto &filename = token.getLocation().getFilename();
      !filename.empty()) {
    std::cout << ", file=\"" << filename << "\"";
  }

  if (token.getErrorMessage()) {
    std::cout << ", error=\"" << *token.getErrorMessage() << "\"";
  }

  std::cout << "}\n";
}

void printTokenStream(const std::vector<tokens::Token> &tokens) {
  std::cout << "Token Stream:\n" << std::string(80, '-') << "\n";

  for (size_t i = 0; i < tokens.size(); ++i) {
    std::cout << std::setw(4) << i << ": ";
    printToken(tokens[i]);
  }

  std::cout << std::string(80, '-') << "\n"
            << "Total tokens: " << tokens.size() << "\n";
}

void printIndent(int level) {
  for (int i = 0; i < level; ++i) {
    std::cout << "  ";
  }
}

std::string getNodeColor(const nodes::NodePtr &node) {
  if (std::dynamic_pointer_cast<nodes::DeclarationNode>(node))
    return GREEN;
  if (std::dynamic_pointer_cast<nodes::ExpressionNode>(node))
    return YELLOW;
  if (std::dynamic_pointer_cast<nodes::TypeNode>(node))
    return BLUE;
  return RESET;
}

std::string getTypeString(tokens::TokenType type) {
  auto it = tokenTypeStrings.find(type);
  return it != tokenTypeStrings.end() ? it->second : "UNKNOWN";
}

void printLocation(const core::SourceLocation &loc) {
  std::cout << "(" << loc.getLine() << ":" << loc.getColumn() << ")";
}

void printASTNode(const nodes::NodePtr &node, int indent) {
  if (!node) {
    printIndent(indent);
    std::cout << RED << "nullptr" << RESET << "\n";
    return;
  }

  printIndent(indent);
  std::cout << getNodeColor(node);

  // Print based on node type
  if (auto varDecl = std::dynamic_pointer_cast<nodes::VarDeclNode>(node)) {
    std::cout << "VarDecl '" << varDecl->getName() << "' ";
    printLocation(varDecl->getLocation());
    std::cout << (varDecl->isConst() ? " const" : "");
    std::cout << "\n";

    printIndent(indent + 1);
    std::cout << BLUE << "Type: ";
    if (varDecl->getType()) {
      if (auto primType = std::dynamic_pointer_cast<nodes::PrimitiveTypeNode>(
              varDecl->getType())) {
        std::cout << getTypeString(primType->getType());
      } else {
        std::cout << "complex_type";
      }
    } else {
      std::cout << "inferred";
    }
    std::cout << RESET << "\n";

    if (varDecl->getInitializer()) {
      printIndent(indent + 1);
      std::cout << YELLOW << "Init:" << RESET << "\n";
      printASTNode(varDecl->getInitializer(), indent + 2);
    }
  } else if (auto literal =
                 std::dynamic_pointer_cast<nodes::LiteralExpressionNode>(
                     node)) {
    std::cout << "Literal '" << literal->getValue() << "' ";
    printLocation(literal->getLocation());
    std::cout << "\n";
  } else if (auto binary =
                 std::dynamic_pointer_cast<nodes::BinaryExpressionNode>(node)) {
    std::cout << "BinaryExpr (" << getTypeString(binary->getExpressionType())
              << ") ";
    printLocation(binary->getLocation());
    std::cout << "\n";
    printASTNode(binary->getLeft(), indent + 1);
    printASTNode(binary->getRight(), indent + 1);
  } else if (auto unary =
                 std::dynamic_pointer_cast<nodes::UnaryExpressionNode>(node)) {
    std::cout << "UnaryExpr (" << getTypeString(unary->getExpressionType())
              << ") ";
    std::cout << (unary->isPrefix() ? "prefix" : "postfix") << " ";
    printLocation(unary->getLocation());
    std::cout << "\n";
    printASTNode(unary->getOperand(), indent + 1);
  } else if (auto ident =
                 std::dynamic_pointer_cast<nodes::IdentifierExpressionNode>(
                     node)) {
    std::cout << "Identifier '" << ident->getName() << "' ";
    printLocation(ident->getLocation());
    std::cout << "\n";
  } else {
    std::cout << "Unknown Node Type ";
    printLocation(node->getLocation());
    std::cout << "\n";
  }

  std::cout << RESET;
}

void printAST(const parser::AST &ast) {
  std::cout << "\nAbstract Syntax Tree:\n" << std::string(80, '-') << "\n";

  const auto &nodes = ast.getNodes();
  if (nodes.empty()) {
    std::cout << RED << "Empty AST" << RESET << "\n";
  } else {
    for (const auto &node : nodes) {
      printASTNode(node);
    }
  }

  std::cout << std::string(80, '-') << "\n";
}