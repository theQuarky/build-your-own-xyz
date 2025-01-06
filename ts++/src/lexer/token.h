#pragma once
#include <string>

namespace lexer {

enum class TokenType {
  // Keywords
  LET,
  CONST,
  FUNCTION,
  RETURN,
  IF,
  ELSE,
  WHILE,
  FOR,
  DO,
  BREAK,
  CONTINUE,

  // Types
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_STRING,
  TYPE_BOOLEAN,
  TYPE_NULL,
  TYPE_UNDEFINED,
  TYPE_IDENTIFIER,

  // Brackets and braces
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  LEFT_BRACKET,
  RIGHT_BRACKET,

  // Literals
  NUMBER_LITERAL,
  STRING_LITERAL,
  BOOLEAN_LITERAL,
  NULL_LITERAL,
  UNDEFINED_LITERAL,

  // Arithmetic Operators
  PLUS,
  MINUS,
  MULTIPLY,
  DIVIDE,
  MODULO,
  POWER,

  // Bitwise Operators
  BITWISE_AND,
  BITWISE_OR,
  BITWISE_XOR,
  BITWISE_NOT,
  LEFT_SHIFT,
  RIGHT_SHIFT,

  // Logical Operators
  AND,
  OR,
  NOT,

  // Comparison Operators
  EQUALS,
  NOT_EQUALS,
  LESS_THAN,
  GREATER_THAN,
  LESS_EQUAL,
  GREATER_EQUAL,

  // Assignment Operators
  ASSIGN,
  PLUS_ASSIGN,
  MINUS_ASSIGN,
  MULTIPLY_ASSIGN,
  DIVIDE_ASSIGN,
  MODULO_ASSIGN,
  AND_ASSIGN,
  OR_ASSIGN,
  XOR_ASSIGN,
  LEFT_SHIFT_ASSIGN,
  RIGHT_SHIFT_ASSIGN,

  // Increment/Decrement
  INCREMENT,
  DECREMENT,

  // Delimiters
  COMMA,
  DOT,
  COLON,
  SEMICOLON,

  // Special tokens
  IDENTIFIER,
  ERROR_TOKEN,
  END_OF_FILE
};

struct Token {
  TokenType type;
  std::string lexeme;
  unsigned int line;
  unsigned int column;
  std::string errorMessage;

  Token(TokenType t, std::string l, unsigned int ln, unsigned int col)
      : type(t), lexeme(std::move(l)), line(ln), column(col) {}

  Token(TokenType t, std::string l, unsigned int ln, unsigned int col,
        std::string err)
      : type(t), lexeme(std::move(l)), line(ln), column(col),
        errorMessage(std::move(err)) {}
};

} // namespace lexer