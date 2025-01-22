#include "log_utils.h"
#include "tokens/token_type.h"
#include <iomanip>
#include <iostream>
#include <unordered_map>

namespace {
std::string getTokenCategory(tokens::TokenType type) {
  if (tokens::isDeclaration(type))
    return "Declaration";
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

    // Memory Management
    {tokens::TokenType::STACK, "STACK"},
    {tokens::TokenType::HEAP, "HEAP"},
    {tokens::TokenType::STATIC, "STATIC"},
    {tokens::TokenType::SHARED, "SHARED"},
    {tokens::TokenType::UNIQUE, "UNIQUE"},
    {tokens::TokenType::WEAK, "WEAK"},

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

    // Operators
    {tokens::TokenType::PLUS, "PLUS"},
    {tokens::TokenType::MINUS, "MINUS"},
    {tokens::TokenType::STAR, "STAR"},
    {tokens::TokenType::SLASH, "SLASH"},
    {tokens::TokenType::PERCENT, "PERCENT"},
    {tokens::TokenType::EQUALS, "EQUALS"},
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

    // compound assignments
    {
        tokens::TokenType::PLUS_EQUALS,
        "+=",
    },
    {tokens::TokenType::MINUS_EQUALS, "-="},
    {tokens::TokenType::STAR_EQUALS, "*="},
    {tokens::TokenType::SLASH_EQUALS, "/="},
    {tokens::TokenType::PERCENT_EQUALS, "%="},
    {tokens::TokenType::AMPERSAND_EQUALS, "&="},
    {tokens::TokenType::PIPE_EQUALS, "|="},
    {tokens::TokenType::CARET_EQUALS, "^="},
    {tokens::TokenType::PLUS_PLUS, "++"},
    {tokens::TokenType::MINUS_MINUS, "--"},

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