#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/nodes/statement_nodes.h"
#include "tokens/stream/token_stream.h"
#include <functional>

namespace visitors {

class BranchStatementVisitor {
public:
  using ExprCallback = std::function<nodes::ExpressionPtr()>;
  using StmtCallback = std::function<nodes::StmtPtr()>;

  BranchStatementVisitor(tokens::TokenStream &tokens,
                         core::ErrorReporter &errorReporter)
      : tokens_(tokens), errorReporter_(errorReporter) {}

  void setCallbacks(ExprCallback exprCb, StmtCallback stmtCb) {
    parseExpr_ = std::move(exprCb);
    parseStmt_ = std::move(stmtCb);
  }

  nodes::StmtPtr parseIfStatement() {
    auto location = tokens_.previous().getLocation();

    if (!consume(tokens::TokenType::LEFT_PAREN, "Expected '(' after 'if'")) {
      return nullptr;
    }

    auto condition = parseExpr_();
    if (!condition)
      return nullptr;

    if (!consume(tokens::TokenType::RIGHT_PAREN,
                 "Expected ')' after condition")) {
      return nullptr;
    }

    auto thenBranch = parseStmt_();
    if (!thenBranch)
      return nullptr;

    nodes::StmtPtr elseBranch;
    if (match(tokens::TokenType::ELSE)) {
      elseBranch = parseStmt_();
      if (!elseBranch)
        return nullptr;
    }

    return std::make_shared<nodes::IfStmtNode>(condition, thenBranch,
                                               elseBranch, location);
  }

  nodes::StmtPtr parseSwitchStatement() {
    // TODO: Implement switch statement parsing
    return nullptr;
  }

private:
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  ExprCallback parseExpr_;
  StmtCallback parseStmt_;

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
};
} // namespace visitors