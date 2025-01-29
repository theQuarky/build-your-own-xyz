/*****************************************************************************
 * File: token_stream.cpp
 * Description: Implementation of TokenStream class providing sequential access
 * to token sequences with lookahead and matching capabilities.
 *****************************************************************************/

#include "token_stream.h"
#include "core/common/common_types.h"

namespace tokens {

/*****************************************************************************
 * Constructor Implementation
 *****************************************************************************/
TokenStream::TokenStream(std::vector<Token> tokens)
    : tokens_(std::move(tokens)), current_(0) {
  // Safety: Ensure stream always ends with EOF token
  if (tokens_.empty() || tokens_.back().getType() != TokenType::END_OF_FILE) {
    tokens_.push_back(
        Token(TokenType::END_OF_FILE, "", core::SourceLocation(0, 0)));
  }
}

/*****************************************************************************
 * Token Access Methods
 *****************************************************************************/
const Token &TokenStream::peek() const {
  // Return EOF token if beyond stream bounds
  if (current_ >= tokens_.size()) {
    return tokens_.back();
  }
  return tokens_[current_];
}

const Token &TokenStream::peekNext(int n) const {
  // Handle negative lookhead as current position
  if (n <= 0) {
    n = 1;
  }
  // Return EOF token if next position beyond stream bounds
  if (current_ + n >= tokens_.size()) {
    return tokens_.back();
  }
  return tokens_[current_ + n];
}

const Token &TokenStream::previous() const {
  // Handle boundary cases
  if (current_ == 0) {
    return tokens_[0]; // At start: return first token
  }
  if (current_ >= tokens_.size()) {
    return tokens_.back(); // Beyond end: return EOF
  }
  return tokens_[current_ - 1];
}

Token TokenStream::advance() {
  // Move forward unless at end
  if (!isAtEnd()) {
    current_++;
  }
  return previous();
}

/*****************************************************************************
 * Stream State Methods
 *****************************************************************************/
bool TokenStream::isAtEnd() const {
  return current_ >= tokens_.size() - 1 ||
         tokens_[current_].getType() == TokenType::END_OF_FILE;
}

bool TokenStream::check(TokenType type) const {
  if (isAtEnd()) {
    return false;
  }
  return peek().getType() == type;
}

bool TokenStream::match(TokenType type) {
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}

bool TokenStream::matchAny(const std::vector<TokenType> &types) {
  // Try to match any of the provided types
  for (const auto &type : types) {
    if (match(type)) {
      return true;
    }
  }
  return false;
}

/*****************************************************************************
 * Position Management
 *****************************************************************************/
void TokenStream::setPosition(size_t position) {
  // Ensure position stays within valid bounds
  if (position < tokens_.size()) {
    current_ = position;
  } else {
    current_ = tokens_.size() - 1; // Clamp to last valid position
  }
}

/*****************************************************************************
 * Error Recovery
 *****************************************************************************/
void TokenStream::synchronize() {
  advance(); // Skip the token where error occurred

  // Continue until we find a synchronization point
  while (!isAtEnd()) {
    // Synchronize at statement boundaries (semicolons)
    if (previous().getType() == TokenType::SEMICOLON) {
      return;
    }

    // Or at the start of major declarations/statements
    switch (peek().getType()) {
    case TokenType::FUNCTION: // Function declaration
    case TokenType::LET:      // Variable declaration
    case TokenType::FOR:      // For loop
    case TokenType::IF:       // If statement
    case TokenType::WHILE:    // While loop
    case TokenType::RETURN:   // Return statement
      return;
    default:
      advance(); // Skip tokens until sync point found
    }
  }
}

} // namespace tokens