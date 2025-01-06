#include "lexer.h"
#include <regex>
#include <sstream>

namespace lexer {

// Initialize static members
const std::regex Lexer::identifierPattern(R"([a-zA-Z_]\w*)");
const std::regex Lexer::numberPattern(R"(-?\d+(\.\d+)?([eE][+-]?\d+)?)");
const std::regex Lexer::stringPattern(R"("([^"\\]|\\.)*")");
const std::regex Lexer::operatorPattern(
    R"(>>>=|<<=|>>=|\+\+|--|&&|\|\||==|!=|<=|>=|\+=|-=|\*=|/=|%=|&=|\|=|\^=|<<|>>|[+\-*/%=&|^~<>!])");

const std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"let", TokenType::LET},
    {"const", TokenType::CONST},
    {"function", TokenType::FUNCTION},
    {"return", TokenType::RETURN},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"int", TokenType::TYPE_INT},
    {"float", TokenType::TYPE_FLOAT},
    {"string", TokenType::TYPE_STRING},
    {"boolean", TokenType::TYPE_BOOLEAN},
    {"null", TokenType::NULL_LITERAL},
    {"undefined", TokenType::UNDEFINED_LITERAL},
    {"true", TokenType::BOOLEAN_LITERAL},
    {"false", TokenType::BOOLEAN_LITERAL}};

const std::unordered_map<std::string, TokenType> Lexer::operators = {
    {">>>", TokenType::RIGHT_SHIFT},
    {"<<<", TokenType::LEFT_SHIFT},
    {">>=", TokenType::RIGHT_SHIFT_ASSIGN},
    {"<<=", TokenType::LEFT_SHIFT_ASSIGN},
    {"++", TokenType::INCREMENT},
    {"--", TokenType::DECREMENT},
    {"&&", TokenType::AND},
    {"||", TokenType::OR},
    {"==", TokenType::EQUALS},
    {"!=", TokenType::NOT_EQUALS},
    {"<=", TokenType::LESS_EQUAL},
    {">=", TokenType::GREATER_EQUAL},
    {"+=", TokenType::PLUS_ASSIGN},
    {"-=", TokenType::MINUS_ASSIGN},
    {"*=", TokenType::MULTIPLY_ASSIGN},
    {"/=", TokenType::DIVIDE_ASSIGN},
    {"%=", TokenType::MODULO_ASSIGN},
    {"&=", TokenType::AND_ASSIGN},
    {"|=", TokenType::OR_ASSIGN},
    {"^=", TokenType::XOR_ASSIGN},
    {"+", TokenType::PLUS},
    {"-", TokenType::MINUS},
    {"*", TokenType::MULTIPLY},
    {"/", TokenType::DIVIDE},
    {"%", TokenType::MODULO},
    {"^", TokenType::POWER},
    {"&", TokenType::BITWISE_AND},
    {"|", TokenType::BITWISE_OR},
    {"~", TokenType::BITWISE_NOT},
    {"!", TokenType::NOT},
    {"<", TokenType::LESS_THAN},
    {">", TokenType::GREATER_THAN},
    {"=", TokenType::ASSIGN}};

Lexer::Lexer(const std::string &source, const std::string &fileName,
             ErrorReporter &errorReporter)
    : source_(source), fileName_(fileName), errorReporter_(errorReporter),
      position_(0), line_(1), column_(1), lastStatementLine_(1),
      statementStarted_(false) {}

std::vector<Token> Lexer::tokenize() {
  while (!isAtEnd()) {
    try {
      skipWhitespace();
      if (!isAtEnd()) {
        scanToken();
      }
    } catch (const std::exception &e) {
      reportError(e.what());
      synchronize(); // Skip to next valid token position
    }
  }

  tokens_.emplace_back(TokenType::END_OF_FILE, "", line_, column_);
  return tokens_;
}

void Lexer::skipWhitespace() {
  while (position_ < source_.length()) {
    char current = source_[position_];

    // Handle spaces and tabs
    if (current == ' ' || current == '\t' || current == '\r') {
      position_++;
      column_++;
      continue;
    }

    // Handle newlines
    if (current == '\n') {
      // Only add semicolon if we have tokens and last token wasn't a semicolon
      if (!tokens_.empty() && tokens_.back().type != TokenType::SEMICOLON) {
        // Check if the next non-whitespace token would make this an invalid
        // line break
        size_t nextPos = position_ + 1;
        while (nextPos < source_.length() &&
               (source_[nextPos] == ' ' || source_[nextPos] == '\t')) {
          nextPos++;
        }

        // If we've hit the end or found another newline, this newline can be a
        // semicolon
        if (nextPos >= source_.length() || source_[nextPos] == '\n' ||
            source_[nextPos] == '}' || source_[nextPos] == ';') {
          tokens_.emplace_back(TokenType::SEMICOLON, ";", line_, column_);
        }
      }
      position_++;
      line_++;
      column_ = 1;
      continue;
    }

    // Handle single-line comments
    if (current == '/' && position_ + 1 < source_.length() &&
        source_[position_ + 1] == '/') {
      position_ += 2;
      while (position_ < source_.length() && source_[position_] != '\n') {
        position_++;
      }
      continue;
    }

    // Handle multi-line comments
    if (current == '/' && position_ + 1 < source_.length() &&
        source_[position_ + 1] == '*') {
      position_ += 2;
      while (position_ + 1 < source_.length() &&
             !(source_[position_] == '*' && source_[position_ + 1] == '/')) {
        if (source_[position_] == '\n') {
          line_++;
          column_ = 1;
        } else {
          column_++;
        }
        position_++;
      }
      if (position_ + 1 < source_.length()) {
        position_ += 2; // Skip closing */
      }
      continue;
    }

    break;
  }
}

bool Lexer::isStatementStart(TokenType type) const {
  return type == TokenType::LET || type == TokenType::CONST ||
         type == TokenType::FUNCTION || type == TokenType::RETURN ||
         type == TokenType::IF || type == TokenType::FOR ||
         type == TokenType::WHILE;
}

void Lexer::scanToken() {
  std::string remaining = source_.substr(position_);
  std::smatch match;

  // Try matching string literals
  if (std::regex_search(remaining, match, stringPattern,
                        std::regex_constants::match_continuous)) {
    std::string str = match.str();
    // Process escape sequences
    std::string processed;
    for (size_t i = 1; i < str.length() - 1;
         ++i) { // Skip opening and closing quotes
      if (str[i] == '\\') {
        i++;
        switch (str[i]) {
        case 'n':
          processed += '\n';
          break;
        case 't':
          processed += '\t';
          break;
        case 'r':
          processed += '\r';
          break;
        case '\\':
          processed += '\\';
          break;
        case '"':
          processed += '"';
          break;
        default:
          reportError("Invalid escape sequence");
          return;
        }
      } else {
        processed += str[i];
      }
    }
    addToken(TokenType::STRING_LITERAL, processed);
    position_ += str.length();
    column_ += str.length();
    return;
  }

  // Try matching numbers
  if (std::regex_search(remaining, match, numberPattern,
                        std::regex_constants::match_continuous)) {
    std::string number = match.str();
    addToken(TokenType::NUMBER_LITERAL, number);
    position_ += number.length();
    column_ += number.length();
    return;
  }

  // Try matching identifiers or keywords
  if (std::regex_search(remaining, match, identifierPattern,
                        std::regex_constants::match_continuous)) {
    std::string identifier = match.str();
    auto it = keywords.find(identifier);
    if (it != keywords.end()) {
      addToken(it->second, identifier);
    } else {
      addToken(TokenType::IDENTIFIER, identifier);
    }
    position_ += identifier.length();
    column_ += identifier.length();
    return;
  }

  // Try matching operators
  if (std::regex_search(remaining, match, operatorPattern,
                        std::regex_constants::match_continuous)) {
    std::string op = match.str();
    auto it = operators.find(op);
    if (it != operators.end()) {
      addToken(it->second, op);
    } else {
      reportError("Unknown operator: " + op);
    }
    position_ += op.length();
    column_ += op.length();
    return;
  }

  // Handle single-character tokens
  char c = source_[position_];
  switch (c) {
  case '(':
    addToken(TokenType::LEFT_PAREN, "(");
    break;
  case ')':
    addToken(TokenType::RIGHT_PAREN, ")");
    break;
  case '{':
    addToken(TokenType::LEFT_BRACE, "{");
    break;
  case '}':
    addToken(TokenType::RIGHT_BRACE, "}");
    break;
  case '[':
    addToken(TokenType::LEFT_BRACKET, "[");
    break;
  case ']':
    addToken(TokenType::RIGHT_BRACKET, "]");
    break;
  case ':':
    addToken(TokenType::COLON, ":");
    break;
  case ';':
    addToken(TokenType::SEMICOLON, ";");
    break;
  case ',':
    addToken(TokenType::COMMA, ",");
    break;
  case '.':
    addToken(TokenType::DOT, ".");
    break;
  default: {
    std::stringstream ss;
    ss << "Unexpected character: '" << c << "'";
    reportError(ss.str());
  }
  }
  position_++;
  column_++;
}

void Lexer::addToken(TokenType type, const std::string &lexeme) {
  // Check for multiple statements on the same line
  if (isStatementStart(type)) {
    if (statementStarted_ && line_ == lastStatementLine_ &&
        (tokens_.empty() || tokens_.back().type != TokenType::SEMICOLON)) {
      std::string msg =
          "Multiple statements on one line require explicit semicolons";
      reportError(msg);
      synchronize();
      return;
    }
    statementStarted_ = true;
    lastStatementLine_ = line_;
  }

  // Reset statement tracking after semicolon
  if (type == TokenType::SEMICOLON) {
    statementStarted_ = false;
  }

  tokens_.emplace_back(type, lexeme, line_, column_);
}

void Lexer::reportError(const std::string &message) {
  errorReporter_.reportError(fileName_, line_, column_, message);
  tokens_.emplace_back(TokenType::ERROR_TOKEN, "", line_, column_, message);
}

void Lexer::synchronize() {
  // Skip until we find a semicolon or newline
  while (!isAtEnd()) {
    char c = source_[position_];
    if (c == ';' || c == '\n') {
      position_++;
      if (c == '\n') {
        line_++;
        column_ = 1;
      } else {
        column_++;
      }
      return;
    }
    position_++;
    column_++;
  }
}

bool Lexer::isAtEnd() const { return position_ >= source_.length(); }

} // namespace lexer