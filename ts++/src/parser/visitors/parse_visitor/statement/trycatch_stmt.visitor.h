#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/nodes/statement_nodes.h"
#include "tokens/stream/token_stream.h"
#include <functional>

namespace visitors {

class TryCatchStatementVisitor {
public:
  using StmtCallback = std::function<nodes::StmtPtr()>;
  using TypeCallback = std::function<nodes::TypePtr()>;

  TryCatchStatementVisitor(tokens::TokenStream &tokens,
                           core::ErrorReporter &errorReporter)
      : tokens_(tokens), errorReporter_(errorReporter) {}

  void setCallbacks(StmtCallback stmtCb, TypeCallback typeCb) {
    parseStmt_ = std::move(stmtCb);
    parseType_ = std::move(typeCb);
  }

  nodes::StmtPtr parseTryStatement() {
    auto location = tokens_.previous().getLocation();

    auto tryBlock = parseStmt_();
    if (!tryBlock)
      return nullptr;

    std::vector<nodes::TryStmtNode::CatchClause> catchClauses;
    nodes::StmtPtr finallyBlock;

    while (match(tokens::TokenType::CATCH)) {
      auto catchClause = parseCatchClause();
      if (!catchClause.body)
        return nullptr;
      catchClauses.push_back(std::move(catchClause));
    }

    if (match(tokens::TokenType::FINALLY)) {
      finallyBlock = parseStmt_();
      if (!finallyBlock)
        return nullptr;
    }

    if (catchClauses.empty() && !finallyBlock) {
      error("Try statement must have at least one catch or finally clause");
      return nullptr;
    }

    return std::make_shared<nodes::TryStmtNode>(
        tryBlock, std::move(catchClauses), finallyBlock, location);
  }

private:
  nodes::TryStmtNode::CatchClause parseCatchClause() {
    nodes::TryStmtNode::CatchClause clause;

    if (!consume(tokens::TokenType::LEFT_PAREN, "Expected '(' after 'catch'")) {
      return clause;
    }

    if (!match(tokens::TokenType::IDENTIFIER)) {
      error("Expected catch parameter name");
      return clause;
    }

    clause.parameter = tokens_.previous().getLexeme();

    if (match(tokens::TokenType::COLON)) {
      clause.parameterType = parseType_();
      if (!clause.parameterType)
        return clause;
    }

    if (!consume(tokens::TokenType::RIGHT_PAREN,
                 "Expected ')' after catch parameter")) {
      return clause;
    }

    clause.body = parseStmt_();
    return clause;
  }

  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  StmtCallback parseStmt_;
  TypeCallback parseType_;

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