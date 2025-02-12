#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/nodes/statement_nodes.h"
#include "parser/visitors/parse_visitor/expression/iexpression_visitor.h"
#include "parser/visitors/parse_visitor/statement/istatement_visitor.h"
#include "tokens/stream/token_stream.h"

namespace visitors {

class ExpressionParseVisitor;
class StatementParseVisitor;

class BranchStatementVisitor {
public:
  BranchStatementVisitor(tokens::TokenStream &tokens,
                         core::ErrorReporter &errorReporter,
                         IExpressionVisitor &exprVisitor,
                         IStatementVisitor &stmtVisitor)
      : tokens_(tokens), errorReporter_(errorReporter),
        exprVisitor_(exprVisitor), stmtVisitor_(stmtVisitor) {}

  nodes::StmtPtr parseIfStatement() {
    auto location = tokens_.previous().getLocation();

    if (!consume(tokens::TokenType::LEFT_PAREN, "Expected '(' after 'if'")) {
      return nullptr;
    }

    auto condition = exprVisitor_.parseExpression();
    if (!condition)
      return nullptr;

    if (!consume(tokens::TokenType::RIGHT_PAREN,
                 "Expected ')' after condition")) {
      return nullptr;
    }

    auto thenBranch = stmtVisitor_.parseStatement();
    if (!thenBranch)
      return nullptr;

    nodes::StmtPtr elseBranch;
    if (match(tokens::TokenType::ELSE)) {
      elseBranch = stmtVisitor_.parseStatement();
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
  inline bool match(tokens::TokenType type) {
    if (check(type)) {
      tokens_.advance();
      return true;
    }
    return false;
  }

  inline bool check(tokens::TokenType type) const {
    return !tokens_.isAtEnd() && tokens_.peek().getType() == type;
  }

  inline bool consume(tokens::TokenType type, const std::string &message) {
    if (check(type)) {
      tokens_.advance();
      return true;
    }
    error(message);
    return false;
  }

  inline void error(const std::string &message) {
    errorReporter_.error(tokens_.peek().getLocation(), message);
  }

  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  IExpressionVisitor &exprVisitor_;
  IStatementVisitor &stmtVisitor_;
  ;
};

} // namespace visitors