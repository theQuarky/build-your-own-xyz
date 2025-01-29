// base_parse_visitor.h
#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/interfaces/base_interface.h"
#include "parser/visitors/parse_visitor/declaration_parse_visitor.h"
#include "parser/visitors/parse_visitor/expression_parse_visitor.h"
#include "tokens/stream/token_stream.h"

namespace visitors {

class BaseParseVisitor : public interface::BaseInterface {
public:
  explicit BaseParseVisitor(tokens::TokenStream &tokens,
                            core::ErrorReporter &errorReporter);
  bool visitParse() override;
  const std::vector<nodes::NodePtr> &getNodes() const { return nodes_; }

private:
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  ExpressionParserVisitor expressionVisitor_;
  DeclarationParseVisitor declarationVisitor_;
  std::vector<nodes::NodePtr> nodes_;

  // Helper methods for parsing different constructs
  nodes::NodePtr parseDeclaration();
  nodes::NodePtr parseStatement();
};
} // namespace visitors