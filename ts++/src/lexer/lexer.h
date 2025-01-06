#pragma once
#include "../core/error_reporter.h"
#include "token.h"
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

namespace lexer {

class Lexer {
private:
  static const std::regex identifierPattern;
  static const std::regex numberPattern;
  static const std::regex stringPattern;
  static const std::regex operatorPattern;
  static const std::unordered_map<std::string, TokenType> keywords;
  static const std::unordered_map<std::string, TokenType> operators;

  std::string source_;
  std::string fileName_;
  ErrorReporter &errorReporter_;
  size_t position_;
  unsigned int line_;
  unsigned int column_;
  std::vector<Token> tokens_;
  unsigned int lastStatementLine_;
  bool statementStarted_;

  void skipWhitespace();
  void scanToken();
  bool isAtEnd() const;
  void synchronize();
  bool isStatementStart(TokenType type) const;
  void addToken(TokenType type, const std::string &lexeme);
  void addToken(TokenType type) {
    addToken(type, source_.substr(position_, 1));
  }
  void reportError(const std::string &message);

public:
  Lexer(const std::string &source, const std::string &fileName,
        ErrorReporter &errorReporter);
  std::vector<Token> tokenize();
};

} // namespace lexer