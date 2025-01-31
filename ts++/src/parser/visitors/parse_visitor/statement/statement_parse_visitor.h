#pragma once
#include "branch_stmt_visitor.h"
#include "core/diagnostics/error_reporter.h"
#include "flowctr_stmt_visitor.h"
#include "loop_stmt_visitor.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/statement_nodes.h"
#include "parser/visitors/parse_visitor/expression/expression_parse_visitor.h"
#include "tokens/stream/token_stream.h"
#include "trycatch_stmt.visitor.h"

namespace visitors {

class StatementParseVisitor {
public:
  StatementParseVisitor(tokens::TokenStream &tokens,
                        core::ErrorReporter &errorReporter,
                        ExpressionParseVisitor &exprVisitor);

  nodes::StmtPtr parseStatement();
  nodes::BlockPtr parseBlock();

private:
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  ExpressionParseVisitor &exprVisitor_;

  // Specialized visitors
  BranchStatementVisitor branchVisitor_;
  LoopStatementVisitor loopVisitor_;
  TryCatchStatementVisitor tryVisitor_;
  FlowControlVisitor flowVisitor_;

  // Parsers for simple statements
  nodes::StmtPtr parseExpressionStatement();
  nodes::StmtPtr parseAssemblyStatement();

  // Utility methods
  bool match(tokens::TokenType type);
  bool check(tokens::TokenType type) const;
  bool consume(tokens::TokenType type, const std::string &message);
  void error(const std::string &message);
  void synchronize();
};
} // namespace visitors