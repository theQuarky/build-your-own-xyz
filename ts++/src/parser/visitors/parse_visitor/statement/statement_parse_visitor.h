#pragma once
#include "branch_stmt_visitor.h"
#include "flowctr_stmt_visitor.h"
#include "loop_stmt_visitor.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/statement_nodes.h"
#include "parser/visitors/parse_visitor/statement/istatement_visitor.h"
#include "trycatch_stmt.visitor.h"
#include <cassert>

namespace visitors {

class ExpressionParseVisitor;

class StatementParseVisitor : public IStatementVisitor {
public:
  StatementParseVisitor(tokens::TokenStream &tokens,
                        core::ErrorReporter &errorReporter,
                        IExpressionVisitor &exprVisitor);

  // Public interface
  nodes::StmtPtr parseStatement() override;
  nodes::BlockPtr parseBlock() override;

private:
  // Statement parsing helpers
  nodes::StmtPtr parseExpressionStatement();
  nodes::StmtPtr parseAssemblyStatement();

  // Utility methods
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

  // Member variables
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  IExpressionVisitor &exprVisitor_;

  // Sub-visitors
  BranchStatementVisitor branchVisitor_;
  LoopStatementVisitor loopVisitor_;
  FlowControlVisitor flowVisitor_;
  TryCatchStatementVisitor tryVisitor_;
};

} // namespace visitors