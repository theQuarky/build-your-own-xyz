#include "expression_parse_visitor.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/visitors/parse_visitor/expression/cast_visitor.h"

namespace visitors {

ExpressionParseVisitor::ExpressionParseVisitor(
    tokens::TokenStream &tokens, core::ErrorReporter &errorReporter)
    : tokens_(tokens), errorReporter_(errorReporter),
      binaryVisitor_(tokens, errorReporter),
      unaryVisitor_(tokens, errorReporter),
      primaryVisitor_(tokens, errorReporter),
      callVisitor_(tokens, errorReporter), castVisitor_(tokens, errorReporter) {

  // Connect visitors

  binaryVisitor_.setUnaryParser(
      [this]() { return unaryVisitor_.parseUnary(); });
  unaryVisitor_.setPrimaryCallback(
      [this]() { return primaryVisitor_.parsePrimary(); });
  primaryVisitor_.setExpressionCallback([this]() { return parseExpression(); });
  callVisitor_.setExpressionCallback([this]() { return parseExpression(); });
}

nodes::ExpressionPtr ExpressionParseVisitor::parseExpression() {
  if (tokens_.peek().getLexeme() == "cast") {
    return castVisitor_.parseCast();
  }

  auto expr = unaryVisitor_.parseUnary();
  if (!expr)
    return nullptr;

  expr = binaryVisitor_.parseBinary(expr, 0);
  if (!expr)
    return nullptr;

  return callVisitor_.parseCallOrMember(expr);
}
} // namespace visitors