#pragma once
#include "class_decl_visitor.h"
#include "func_decl_visitor.h"
#include "ideclaration_visitor.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/visitors/parse_visitor/expression/iexpression_visitor.h"
#include "var_decl_visitor.h"

namespace visitors {

class ExpressionParseVisitor;
class StatementParseVisitor;

class DeclarationParseVisitor : public IDeclarationVisitor {
public:
  DeclarationParseVisitor(tokens::TokenStream &tokens,
                          core::ErrorReporter &errorReporter,
                          IExpressionVisitor &exprVisitor,
                          IStatementVisitor &stmtVisitor);

  nodes::DeclPtr parseDeclaration() override;
  nodes::TypePtr parseType() override;
  nodes::BlockPtr parseBlock() override;

private:
  // Utility methods
  bool match(tokens::TokenType type);
  bool check(tokens::TokenType type) const;
  void error(const std::string &message);
  bool consume(tokens::TokenType type, const std::string &message);

  // Resources
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  IExpressionVisitor &exprVisitor_;
  IStatementVisitor &stmtVisitor_;

  // Sub-visitors
  VariableDeclarationVisitor varDeclVisitor_;
  FunctionDeclarationVisitor funcDeclVisitor_;
  ClassDeclarationVisitor classDeclVisitor_;

  // Helper methods
  tokens::TokenType parseStorageClass();
  std::vector<nodes::AttributePtr> parseAttributeList();
  nodes::AttributePtr parseAttribute();
  nodes::TypePtr parsePrimaryType();
  nodes::TypePtr parseTemplateType(const core::SourceLocation &location);
  nodes::TypePtr parseSmartPointerType(const core::SourceLocation &location);
  nodes::TypePtr parseUnionType(nodes::TypePtr leftType,
                                const core::SourceLocation &location);
  nodes::TypePtr parseArrayType(nodes::TypePtr elementType,
                                const core::SourceLocation &location);
  nodes::TypePtr parsePointerType(nodes::TypePtr baseType,
                                  const core::SourceLocation &location);
  bool parseFunctionModifiers(std::vector<tokens::TokenType> &modifiers);
  bool parseClassModifiers(std::vector<tokens::TokenType> &modifiers);
};
} // namespace visitors