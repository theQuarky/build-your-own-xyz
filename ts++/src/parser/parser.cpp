#include "parser.h"
#include <iostream>
#include <ostream>

namespace parser {

Parser::Parser(std::vector<lexer::Token> tokens, ErrorReporter &errorReporter)
    : tokens_(std::move(tokens)), errorReporter_(errorReporter), current_(0) {}

std::vector<ast::StmtPtr> Parser::parse() {
  std::vector<ast::StmtPtr> statements;
  while (!isAtEnd()) {
    try {
      statements.push_back(declaration());
    } catch (const ParserError &e) {
      // Report the error instead of silently catching it
      errorReporter_.reportError("parser", peek().line, peek().column,
                                 e.what());
      synchronize();
    }
  }
  return statements;
}

ast::ExprPtr Parser::expression() { return assignment(); }

ast::ExprPtr Parser::assignment() {
  auto expr = logicalOr();

  if (match(lexer::TokenType::ASSIGN)) {
    auto equals = previous();
    auto value = assignment();

    if (auto var = std::dynamic_pointer_cast<ast::Variable>(expr)) {
      return std::make_shared<ast::Assignment>(equals, std::move(expr),
                                               std::move(value));
    }
    error("Invalid assignment target.");
  } else if (match(lexer::TokenType::OR_ASSIGN) ||
             match(lexer::TokenType::AND_ASSIGN) ||
             match(lexer::TokenType::XOR_ASSIGN) ||
             match(lexer::TokenType::LEFT_SHIFT_ASSIGN) ||
             match(lexer::TokenType::RIGHT_SHIFT_ASSIGN) ||
             match(lexer::TokenType::PLUS_ASSIGN) ||
             match(lexer::TokenType::MINUS_ASSIGN) ||
             match(lexer::TokenType::MULTIPLY_ASSIGN) ||
             match(lexer::TokenType::DIVIDE_ASSIGN) ||
             match(lexer::TokenType::MODULO_ASSIGN)) {
    auto op = previous();
    auto value = assignment();

    if (auto var = std::dynamic_pointer_cast<ast::Variable>(expr)) {
      return std::make_shared<ast::CompoundAssignment>(
          op, std::move(expr), std::move(value), op.lexeme);
    }
    error("Invalid compound assignment target.");
  }

  return expr;
}

// Add bitwise operations between logical and equality
ast::ExprPtr Parser::logicalAnd() {
  auto expr = bitOr();
  while (match(lexer::TokenType::AND)) {
    auto op = previous();
    auto right = bitOr();
    expr =
        std::make_shared<ast::BinaryOp>(op, std::move(expr), std::move(right));
  }
  return expr;
}

ast::ExprPtr Parser::bitOr() {
  auto expr = bitXor();
  while (match(lexer::TokenType::BITWISE_OR)) {
    auto op = previous();
    auto right = bitXor();
    expr =
        std::make_shared<ast::BinaryOp>(op, std::move(expr), std::move(right));
  }
  return expr;
}

ast::ExprPtr Parser::bitXor() {
  auto expr = bitAnd();
  while (match(lexer::TokenType::BITWISE_XOR)) {
    auto op = previous();
    auto right = bitAnd();
    expr =
        std::make_shared<ast::BinaryOp>(op, std::move(expr), std::move(right));
  }
  return expr;
}

ast::ExprPtr Parser::bitAnd() {
  auto expr = shift();
  while (match(lexer::TokenType::BITWISE_AND)) {
    auto op = previous();
    auto right = shift();
    expr =
        std::make_shared<ast::BinaryOp>(op, std::move(expr), std::move(right));
  }
  return expr;
}

ast::ExprPtr Parser::shift() {
  auto expr = equality();
  while (match(lexer::TokenType::LEFT_SHIFT) ||
         match(lexer::TokenType::RIGHT_SHIFT)) {
    auto op = previous();
    auto right = equality();
    expr =
        std::make_shared<ast::BinaryOp>(op, std::move(expr), std::move(right));
  }
  return expr;
}

ast::ExprPtr Parser::logicalOr() {
  auto expr = logicalAnd();

  while (match(lexer::TokenType::OR)) {
    auto op = previous();
    auto right = logicalAnd();
    expr =
        std::make_shared<ast::BinaryOp>(op, std::move(expr), std::move(right));
  }
  return expr;
}

// ast::ExprPtr Parser::logicalAnd() {
//   auto expr = equality();

//   while (match(lexer::TokenType::AND)) {
//     auto op = previous();
//     auto right = equality();
//     expr =
//         std::make_shared<ast::BinaryOp>(op, std::move(expr),
//         std::move(right));
//   }
//   return expr;
// }

ast::ExprPtr Parser::equality() {
  auto expr = comparison();

  while (match(lexer::TokenType::EQUALS) ||
         match(lexer::TokenType::NOT_EQUALS)) {
    auto op = previous();
    auto right = comparison();
    expr =
        std::make_shared<ast::BinaryOp>(op, std::move(expr), std::move(right));
  }
  return expr;
}

ast::ExprPtr Parser::comparison() {
  auto expr = term();

  while (match(lexer::TokenType::LESS_THAN) ||
         match(lexer::TokenType::GREATER_THAN) ||
         match(lexer::TokenType::LESS_EQUAL) ||
         match(lexer::TokenType::GREATER_EQUAL)) {
    auto op = previous();
    auto right = term();
    expr =
        std::make_shared<ast::BinaryOp>(op, std::move(expr), std::move(right));
  }
  return expr;
}

ast::ExprPtr Parser::term() {
  auto expr = factor();

  while (match(lexer::TokenType::PLUS) || match(lexer::TokenType::MINUS)) {
    auto op = previous();
    auto right = factor();
    expr =
        std::make_shared<ast::BinaryOp>(op, std::move(expr), std::move(right));
  }
  return expr;
}

ast::ExprPtr Parser::factor() {
  auto expr = unary();

  while (match(lexer::TokenType::MULTIPLY) || match(lexer::TokenType::DIVIDE) ||
         match(lexer::TokenType::MODULO)) {
    auto op = previous();
    auto right = unary();
    expr =
        std::make_shared<ast::BinaryOp>(op, std::move(expr), std::move(right));
  }
  return expr;
}

ast::ExprPtr Parser::unary() {
  if (match(lexer::TokenType::NOT) || match(lexer::TokenType::MINUS) ||
      match(lexer::TokenType::BITWISE_NOT)) {
    auto op = previous();
    auto right = unary();
    return std::make_shared<ast::UnaryOp>(op, std::move(right));
  }
  return call();
}

ast::ExprPtr Parser::call() {
  auto expr = primary();

  while (true) {
    if (match(lexer::TokenType::LEFT_PAREN)) {
      std::vector<ast::ExprPtr> arguments;
      if (!check(lexer::TokenType::RIGHT_PAREN)) {
        do {
          arguments.push_back(expression());
        } while (match(lexer::TokenType::COMMA));
      }
      consume(lexer::TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
      expr = std::make_shared<ast::Call>(previous(), std::move(expr),
                                         std::move(arguments));
    } else {
      break;
    }
  }
  return expr;
}

ast::ExprPtr Parser::primary() {
  if (match(lexer::TokenType::NUMBER_LITERAL) ||
      match(lexer::TokenType::STRING_LITERAL) ||
      match(lexer::TokenType::BOOLEAN_LITERAL)) {
    return std::make_shared<ast::Literal>(previous(), previous().lexeme);
  }

  if (match(lexer::TokenType::NULL_LITERAL)) {
    return std::make_shared<ast::Literal>(previous(), "null");
  }

  if (match(lexer::TokenType::UNDEFINED_LITERAL)) {
    return std::make_shared<ast::Literal>(previous(), "undefined");
  }

  if (match(lexer::TokenType::IDENTIFIER)) {
    return std::make_shared<ast::Variable>(previous());
  }

  if (match(lexer::TokenType::LEFT_PAREN)) {
    auto expr = expression();
    consume(lexer::TokenType::RIGHT_PAREN, "Expect ')' after expression.");
    return expr;
  }

  throw ParserError("Expect expression.");
}

ast::StmtPtr Parser::declaration() {
  try {
    if (match(lexer::TokenType::LET) || match(lexer::TokenType::CONST)) {
      return varDeclaration();
    }
    if (match(lexer::TokenType::FUNCTION)) {
      return functionDeclaration();
    }
    return statement();
  } catch (const ParserError &e) {
    synchronize();
    return nullptr;
  }
}
ast::StmtPtr Parser::varDeclaration() {
  bool isConst = previous().type == lexer::TokenType::CONST;
  auto name = consume(lexer::TokenType::IDENTIFIER, "Expect variable name.");

  consume(lexer::TokenType::COLON, "Expect ':' after variable name.");
  auto varType = type();

  ast::ExprPtr initializer = nullptr;
  if (match(lexer::TokenType::ASSIGN)) {
    initializer = expression();
  }

  auto currentToken = peek();
  auto prevToken = previous();

  // Semicolon is optional at end of line or end of file
  if (currentToken.line > prevToken.line ||
      currentToken.type == lexer::TokenType::END_OF_FILE) {
    return std::make_shared<ast::VarDeclaration>(
        name, name.lexeme, std::move(varType), std::move(initializer), isConst);
  }

  consume(lexer::TokenType::SEMICOLON,
          "Expect ';' after variable declaration on same line.");
  return std::make_shared<ast::VarDeclaration>(
      name, name.lexeme, std::move(varType), std::move(initializer), isConst);
}

ast::StmtPtr Parser::functionDeclaration() {
  auto name = consume(lexer::TokenType::IDENTIFIER, "Expect function name.");
  consume(lexer::TokenType::LEFT_PAREN, "Expect '(' after function name.");

  std::vector<std::pair<std::string, ast::TypePtr>> parameters;
  if (!check(lexer::TokenType::RIGHT_PAREN)) {
    do {
      auto paramName =
          consume(lexer::TokenType::IDENTIFIER, "Expect parameter name.");
      consume(lexer::TokenType::COLON, "Expect ':' after parameter name.");
      auto paramType = type();
      parameters.emplace_back(paramName.lexeme, std::move(paramType));
    } while (match(lexer::TokenType::COMMA));
  }

  consume(lexer::TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
  consume(lexer::TokenType::COLON, "Expect ':' before return type.");
  auto returnType = type();

  consume(lexer::TokenType::LEFT_BRACE, "Expect '{' before function body.");
  auto body = block();

  return std::make_shared<ast::FunctionDeclaration>(
      name, name.lexeme, std::move(parameters), std::move(returnType),
      std::move(body));
}

ast::StmtPtr Parser::ifStatement() {
  consume(lexer::TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
  auto condition = expression();
  consume(lexer::TokenType::RIGHT_PAREN, "Expect ')' after if condition.");

  auto thenBranch = statement();
  ast::StmtPtr elseBranch = nullptr;

  if (match(lexer::TokenType::ELSE)) {
    elseBranch = statement();
  }

  return std::make_shared<ast::If>(previous(), std::move(condition),
                                   std::move(thenBranch),
                                   std::move(elseBranch));
}

ast::StmtPtr Parser::returnStatement() {
  auto keyword = previous();
  ast::ExprPtr value = nullptr;

  if (!check(lexer::TokenType::SEMICOLON)) {
    value = expression();
  }

  consume(lexer::TokenType::SEMICOLON, "Expect ';' after return value.");
  return std::make_shared<ast::Return>(keyword, std::move(value));
}

bool Parser::isBlockEnd() const {
  return peek().type == lexer::TokenType::RIGHT_BRACE || isAtEnd();
}

std::vector<ast::StmtPtr> Parser::block() {
  std::vector<ast::StmtPtr> statements;

  while (!isBlockEnd()) {
    auto stmt = declaration();
    if (stmt) {
      statements.push_back(std::move(stmt));
    }
  }

  consume(lexer::TokenType::RIGHT_BRACE, "Expect '}' after block.");
  return statements;
}

ast::StmtPtr Parser::statement() {
  if (match(lexer::TokenType::IF))
    return ifStatement();
  if (match(lexer::TokenType::RETURN))
    return returnStatement();
  if (match(lexer::TokenType::LEFT_BRACE)) {
    return std::make_shared<ast::Block>(previous(), block());
  }
  return expressionStatement();
}

ast::StmtPtr Parser::expressionStatement() {
  auto expr = expression();

  // Check if we're at the end of line or file
  if (isAtEnd() || peek().line > previous().line) {
    return std::make_shared<ast::ExpressionStmt>(previous(), std::move(expr));
  }

  // If we're still on the same line, require a semicolon
  if (!match(lexer::TokenType::SEMICOLON)) {
    throw ParserError("Expect ';' after expression on same line.");
  }
  return std::make_shared<ast::ExpressionStmt>(previous(), std::move(expr));
}

ast::TypePtr Parser::type() {
  if (match(lexer::TokenType::TYPE_INT) ||
      match(lexer::TokenType::TYPE_FLOAT) ||
      match(lexer::TokenType::TYPE_STRING) ||
      match(lexer::TokenType::TYPE_BOOLEAN)) {
    return std::make_shared<ast::BasicType>(previous(), previous().lexeme);
  }
  throw ParserError("Expect type.");
}

bool Parser::match(lexer::TokenType type) {
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}

bool Parser::check(lexer::TokenType type) const {
  if (isAtEnd())
    return false;
  return peek().type == type;
}

lexer::Token Parser::advance() {
  if (!isAtEnd())
    current_++;
  return previous();
}

lexer::Token Parser::peek() const { return tokens_[current_]; }

lexer::Token Parser::previous() const { return tokens_[current_ - 1]; }

bool Parser::isAtEnd() const {
  return peek().type == lexer::TokenType::END_OF_FILE;
}

lexer::Token Parser::consume(lexer::TokenType type,
                             const std::string &message) {
  if (check(type))
    return advance();
  throw ParserError(message);
}

void Parser::synchronize() {
  advance();

  while (!isAtEnd()) {
    if (previous().type == lexer::TokenType::SEMICOLON ||
        previous().type == lexer::TokenType::RIGHT_BRACE) {
      return;
    }

    switch (peek().type) {
    case lexer::TokenType::FUNCTION:
    case lexer::TokenType::LET:
    case lexer::TokenType::CONST:
    case lexer::TokenType::IF:
    case lexer::TokenType::RETURN:
    case lexer::TokenType::FOR:
    case lexer::TokenType::WHILE:
      return;
    default:
      advance();
    }
  }
}

void Parser::error(const std::string &message) {
  errorReporter_.reportError("parser", peek().line, peek().column, message);
  throw ParserError(message);
}

} // namespace parser