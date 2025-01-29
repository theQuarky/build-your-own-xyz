/*****************************************************************************
 * File: identifier_scanner.cpp
 *
 * Description:
 *   Implementation of identifier and keyword scanning functionality.
 *   Handles lexical analysis of identifiers, keywords, and attributes.
 *****************************************************************************/

#include "identifier_scanner.h"
#include "lexer/patterns/lexer_patterns.h"
#include "tokens/token_type.h"
#include "tokens/tokens.h"

namespace lexer {

namespace {
// Cache the keyword map as a static member for efficiency
const std::unordered_map<std::string_view, tokens::TokenType> &
getKeywordMapImpl() {
  static const std::unordered_map<std::string_view, tokens::TokenType>
      keywords = {{"let", tokens::TokenType::LET},
                  {"const", tokens::TokenType::CONST},
                  {"function", tokens::TokenType::FUNCTION},
                  {"class", tokens::TokenType::CLASS},
                  {"interface", tokens::TokenType::INTERFACE},
                  {"enum", tokens::TokenType::ENUM},
                  {"typedef", tokens::TokenType::TYPEDEF},
                  {"namespace", tokens::TokenType::NAMESPACE},
                  {"if", tokens::TokenType::IF},
                  {"else", tokens::TokenType::ELSE},
                  {"for", tokens::TokenType::FOR},
                  {"while", tokens::TokenType::WHILE},
                  {"do", tokens::TokenType::DO},
                  {"break", tokens::TokenType::BREAK},
                  {"continue", tokens::TokenType::CONTINUE},
                  {"return", tokens::TokenType::RETURN},
                  {"true", tokens::TokenType::TRUE},
                  {"false", tokens::TokenType::FALSE},
                  {"null", tokens::TokenType::NULL_VALUE},
                  {"undefined", tokens::TokenType::UNDEFINED},
                  {"this", tokens::TokenType::THIS},
                  {"void", tokens::TokenType::VOID},
                  {"int", tokens::TokenType::INT},
                  {"float", tokens::TokenType::FLOAT},
                  {"boolean", tokens::TokenType::BOOLEAN},
                  {"string", tokens::TokenType::STRING},
                  {"try", tokens::TokenType::TRY},
                  {"catch", tokens::TokenType::CATCH},
                  {"switch", tokens::TokenType::SWITCH},
                  {"case", tokens::TokenType::CASE},
                  {"default", tokens::TokenType::DEFAULT},
                  {"extends", tokens::TokenType::EXTENDS},
                  {"implements", tokens::TokenType::IMPLEMENTS},
                  {"public", tokens::TokenType::PUBLIC},
                  {"private", tokens::TokenType::PRIVATE},
                  {"protected", tokens::TokenType::PROTECTED},
                  {"new", tokens::TokenType::NEW},
                  {"throw", tokens::TokenType::THROW},
                  {"typeof", tokens::TokenType::TYPEOF}

      };
  return keywords;
}
} // namespace

/*****************************************************************************
 * Construction
 *****************************************************************************/
IdentifierScanner::IdentifierScanner(std::shared_ptr<LexerState> state)
    : ScannerBase(std::move(state)) {}

/*****************************************************************************
 * Public Interface Implementation
 *****************************************************************************/
tokens::Token IdentifierScanner::scan() {
  size_t start = state_->getPosition();

  // Scan identifier characters
  while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
    advance();
  }

  // Calculate length
  size_t length = state_->getPosition() - start;

  // Get source string_view and extract lexeme for validation and type checking
  std::string_view source = state_->getSource();
  std::string_view lexeme = source.substr(start, length);

  if (!validateIdentifier(lexeme)) {
    return makeErrorToken("Invalid identifier");
  }

  // Check if it's a keyword and get its type
  tokens::TokenType type = identifierType(lexeme);

  // Create token with determined type
  return makeToken(type, start, length);
}

tokens::Token IdentifierScanner::scanAttribute() {
  size_t start = state_->getPosition();
  advance(); // Skip #

  // Scan attribute name
  while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
    advance();
  }

  // Create string directly instead of string_view
  const std::string &source = state_->getSource();
  std::string fullAttr = source.substr(start, state_->getPosition() - start);

  // Check if it's a valid attribute name
  if (!lexer::LexerPatterns::isValidAttribute(fullAttr)) {
    return makeErrorToken("Unknown attribute");
  }

  return makeToken(tokens::TokenType::ATTRIBUTE, start,
                   state_->getPosition() - start);
}

/*****************************************************************************
 * Private Helper Methods
 *****************************************************************************/
const std::unordered_map<std::string_view, tokens::TokenType> &
IdentifierScanner::getKeywordMap() {
  return getKeywordMapImpl();
}

bool IdentifierScanner::validateIdentifier(std::string_view lexeme) {
  if (lexeme.empty()) {
    return false;
  }

  // First character must be letter or underscore
  if (!std::isalpha(lexeme[0]) && lexeme[0] != '_') {
    return false;
  }

  // Remaining characters must be alphanumeric or underscore
  return std::all_of(lexeme.begin() + 1, lexeme.end(),
                     [](char c) { return std::isalnum(c) || c == '_'; });
}

tokens::TokenType IdentifierScanner::identifierType(std::string_view lexeme) {
  auto &keywords = getKeywordMap();
  auto it = keywords.find(lexeme);
  return it != keywords.end() ? it->second : tokens::TokenType::IDENTIFIER;
}

} // namespace lexer