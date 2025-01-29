#pragma once
#include "parser/nodes/expression_nodes.h"
#include "tokens/stream/token_stream.h"

namespace visitors {

class ExpressionParserVisitor {
public:
  explicit ExpressionParserVisitor(tokens::TokenStream &tokens);

  // Main entry point for expression parsing
  nodes::ExpressionPtr parseExpression();

protected:
  // Expression parsing methods following grammar precedence
  nodes::ExpressionPtr parseAssignmentExpr();
  nodes::ExpressionPtr parseConditionalExpr();
  nodes::ExpressionPtr parseLogicalOrExpr();
  nodes::ExpressionPtr parseLogicalAndExpr();
  nodes::ExpressionPtr parseBitwiseOrExpr();
  nodes::ExpressionPtr parseBitwiseXorExpr();
  nodes::ExpressionPtr parseBitwiseAndExpr();
  nodes::ExpressionPtr parseEqualityExpr();
  nodes::ExpressionPtr parseRelationalExpr();
  nodes::ExpressionPtr parseShiftExpr();
  nodes::ExpressionPtr parseAdditiveExpr();
  nodes::ExpressionPtr parseMultiplicativeExpr();
  nodes::ExpressionPtr parseUnaryExpr();
  nodes::ExpressionPtr parsePostfixExpr();
  nodes::ExpressionPtr parsePrimaryExpr();

  // Helper methods
  nodes::ExpressionPtr parseArrayLiteral();
  nodes::ExpressionPtr parseCompileTimeExpr();
  nodes::ExpressionPtr parseNewExpression();
  nodes::ExpressionPtr parseFunctionCall(nodes::ExpressionPtr callee);
  nodes::ExpressionPtr parseMemberAccess(nodes::ExpressionPtr object);

private:
  tokens::TokenStream &tokens_;

  // Helper methods for token processing
  bool match(tokens::TokenType type);
  bool matchAny(const std::vector<tokens::TokenType> &types);
  const tokens::Token &peek() const;
  const tokens::Token &advance();
  bool check(tokens::TokenType type) const;

  // Error handling
  nodes::ExpressionPtr error(const std::string &message);
};

} // namespace visitors