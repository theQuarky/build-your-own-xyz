#pragma once
#include "binary_visitor.h"
#include "parser/nodes/type_nodes.h"
#include "parser/visitors/parse_visitor/expression/call_visitor.h"
#include "parser/visitors/parse_visitor/expression/cast_visitor.h"
#include "primary_visitor.h"
#include "unary_visitor.h"

namespace visitors {

class ExpressionParseVisitor {
public:
  ExpressionParseVisitor(tokens::TokenStream &tokens,
                         core::ErrorReporter &errorReporter);
  nodes::ExpressionPtr parseExpression();
  nodes::TypePtr parseType();

private:
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;

  BinaryExpressionVisitor binaryVisitor_;
  UnaryExpressionVisitor unaryVisitor_;
  PrimaryExpressionVisitor primaryVisitor_;
  CallExpressionVisitor callVisitor_;
  CastExpressionVisitor castVisitor_;
};

} // namespace visitors