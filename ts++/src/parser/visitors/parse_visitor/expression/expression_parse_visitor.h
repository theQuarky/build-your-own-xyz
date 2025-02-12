#pragma once
#include "binary_visitor.h"
#include "call_visitor.h"
#include "cast_visitor.h"
#include "iexpression_visitor.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/nodes/type_nodes.h"
#include "primary_visitor.h"
#include "unary_visitor.h"
#include <cassert>

namespace visitors {

class ExpressionParseVisitor : public IExpressionVisitor {
public:
  ExpressionParseVisitor(tokens::TokenStream &tokens,
                         core::ErrorReporter &errorReporter);

  // Main public interface
  nodes::ExpressionPtr parseExpression() override;
  nodes::ExpressionPtr parsePrimary() override;
  nodes::ExpressionPtr parseUnary() override;
  nodes::TypePtr parseType() override;

  // Friend declarations for sub-visitors
  friend class BinaryExpressionVisitor;
  friend class UnaryExpressionVisitor;
  friend class PrimaryExpressionVisitor;
  friend class CallExpressionVisitor;
  friend class CastExpressionVisitor;

private:

  // Utility methods
  bool match(tokens::TokenType type);
  bool check(tokens::TokenType type) const;
  void error(const std::string &message);

  // Member variables
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;

  // Sub-visitors
  BinaryExpressionVisitor binaryVisitor_;
  UnaryExpressionVisitor unaryVisitor_;
  PrimaryExpressionVisitor primaryVisitor_;
  CallExpressionVisitor callVisitor_;
  CastExpressionVisitor castVisitor_;
};

} // namespace visitors
