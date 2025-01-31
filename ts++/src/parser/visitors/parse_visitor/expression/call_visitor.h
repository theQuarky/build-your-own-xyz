#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/nodes/expression_nodes.h"
#include "tokens/stream/token_stream.h"
#include <functional>

namespace visitors {

class CallExpressionVisitor {
public:
  using ExpressionCallback = std::function<nodes::ExpressionPtr()>;

  CallExpressionVisitor(tokens::TokenStream &tokens,
                        core::ErrorReporter &errorReporter)
      : tokens_(tokens), errorReporter_(errorReporter) {}

  void setExpressionCallback(ExpressionCallback callback) {
    parseExpressionCallback = std::move(callback);
  }
  nodes::ExpressionPtr parseCallOrMember(nodes::ExpressionPtr expr) {
    while (true) {
      if (match(tokens::TokenType::LEFT_PAREN)) {
        expr = finishCall(expr);
      } else if (match(tokens::TokenType::DOT)) {
        if (!match(tokens::TokenType::IDENTIFIER)) {
          error("Expected property name after '.'");
          return nullptr;
        }
        expr = std::make_shared<nodes::MemberExpressionNode>(
            tokens_.previous().getLocation(), expr,
            tokens_.previous().getLexeme(), false);
      } else if (match(tokens::TokenType::AT)) {
        if (!match(tokens::TokenType::IDENTIFIER)) {
          error("Expected property name after '@'");
          return nullptr;
        }
        expr = std::make_shared<nodes::MemberExpressionNode>(
            tokens_.previous().getLocation(), expr,
            tokens_.previous().getLexeme(), true);
      } else {
        break;
      }
    }
    return expr;
  }

private:
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;

  nodes::ExpressionPtr finishCall(nodes::ExpressionPtr callee) {
    std::vector<nodes::ExpressionPtr> arguments;

    if (!check(tokens::TokenType::RIGHT_PAREN)) {
      do {
        if (arguments.size() >= 255) {
          error("Cannot have more than 255 arguments");
          return nullptr;
        }

        auto arg = parseExpression();
        if (!arg)
          return nullptr;
        arguments.push_back(std::move(arg));
      } while (match(tokens::TokenType::COMMA));
    }

    if (!consume(tokens::TokenType::RIGHT_PAREN,
                 "Expected ')' after arguments")) {
      return nullptr;
    }

    return std::make_shared<nodes::CallExpressionNode>(
        callee->getLocation(), callee, std::move(arguments));
  }

  bool match(tokens::TokenType type) {
    if (check(type)) {
      tokens_.advance();
      return true;
    }
    return false;
  }

  bool check(tokens::TokenType type) const {
    return !tokens_.isAtEnd() && tokens_.peek().getType() == type;
  }

  bool consume(tokens::TokenType type, const std::string &message) {
    if (check(type)) {
      tokens_.advance();
      return true;
    }
    error(message);
    return false;
  }

  void error(const std::string &message) {
    errorReporter_.error(tokens_.peek().getLocation(), message);
  }

  // Forward declaration - will be provided by main expression visitor
  ExpressionCallback parseExpressionCallback;
  nodes::ExpressionPtr parseExpression() { return parseExpressionCallback(); }
};
} // namespace visitors