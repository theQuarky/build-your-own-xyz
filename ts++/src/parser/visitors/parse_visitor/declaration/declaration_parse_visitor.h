#pragma once
#include "class_decl_visitor.h"
#include "core/diagnostics/error_reporter.h"
#include "func_decl_visitor.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/visitors/parse_visitor/statement/statement_parse_visitor.h"
#include "tokens/stream/token_stream.h"
#include "var_decl_visitor.h"

namespace visitors {

class DeclarationParseVisitor {
public:
  DeclarationParseVisitor(tokens::TokenStream &tokens,
                          core::ErrorReporter &errorReporter,
                          ExpressionParseVisitor &exprVisitor,
                          StatementParseVisitor &stmtVisitor);

  nodes::DeclPtr parseDeclaration();
  nodes::TypePtr parseType();
  nodes::BlockPtr parseBlock();

private:
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  ExpressionParseVisitor &exprVisitor_;
  StatementParseVisitor &stmtVisitor_;

  VariableDeclarationVisitor varDeclVisitor_;
  FunctionDeclarationVisitor funcDeclVisitor_;
  ClassDeclarationVisitor classDeclVisitor_;

  // Helper methods
  tokens::TokenType parseStorageClass();
  nodes::TypePtr parsePrimaryType();
  nodes::TypePtr parseTypeModifiers(nodes::TypePtr baseType);
  std::vector<nodes::AttributePtr> parseAttributeList();
  nodes::AttributePtr parseAttribute();
  bool matchAny(const std::vector<tokens::TokenType> &types);
  bool match(tokens::TokenType type);
  bool check(tokens::TokenType type) const;
  bool consume(tokens::TokenType type, const std::string &message);
  void error(const std::string &message);
  void synchronize();
};
} // namespace visitors