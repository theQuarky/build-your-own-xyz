
// base_parse_visitor.h
#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/interfaces/base_interface.h"
#include "parser/visitors/parse_visitor/declaration/declaration_parse_visitor.h"
#include "parser/visitors/parse_visitor/expression/expression_parse_visitor.h"
#include "parser/visitors/parse_visitor/statement/statement_parse_visitor.h"
#include "tokens/stream/token_stream.h"
#include <vector>

namespace visitors {

/**
 * Base parse visitor that coordinates parsing operations
 */
class BaseParseVisitor : public interface::BaseInterface {
public:
  explicit BaseParseVisitor(tokens::TokenStream &tokens,
                            core::ErrorReporter &errorReporter);

  bool visitParse() override;
  const std::vector<nodes::NodePtr> &getNodes() const { return nodes_; }

protected:
  // Common utilities for parsing
  bool match(tokens::TokenType type);
  bool check(tokens::TokenType type) const;
  void error(const std::string &message);
  void synchronize();

  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;

private:
  // Main visitor components
  ExpressionParseVisitor expressionVisitor_;
  StatementParseVisitor statementVisitor_;
  DeclarationParseVisitor declarationVisitor_;
  std::vector<nodes::NodePtr> nodes_;

  // Parse handlers
  nodes::NodePtr parseDeclaration();
  nodes::NodePtr parseStatement();
};
} // namespace visitors