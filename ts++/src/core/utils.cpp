#include "./utils.h"
#include <iomanip>
#include <iostream>
#include <string>

// Helper function to get token type name
std::string getTokenTypeName(lexer::TokenType type) {
  switch (type) {
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
  case lexer::TokenType::IDENTIFIER:
    return "IDENTIFIER";
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
  case lexer::TokenType::TYPE_INT:
    return "TYPE_INT";
  case lexer::TokenType::TYPE_FLOAT:
    return "TYPE_FLOAT";
  case lexer::TokenType::TYPE_STRING:
    return "TYPE_STRING";
  case lexer::TokenType::TYPE_BOOLEAN:
    return "TYPE_BOOLEAN";
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
  case lexer::TokenType::ASSIGN:
    return "ASSIGN";
  case lexer::TokenType::EQUALS:
    return "EQUALS";
  case lexer::TokenType::NOT_EQUALS:
    return "NOT_EQUALS";
  case lexer::TokenType::LESS_THAN:
    return "LESS_THAN";
  case lexer::TokenType::GREATER_THAN:
    return "GREATER_THAN";
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