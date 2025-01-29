// declaration_parse_visitor.h
#pragma once
#include "core/diagnostics/error_reporter.h"
#include "expression_parse_visitor.h"
#include "parser/nodes/declaration_nodes.h"
#include "tokens/stream/token_stream.h"

namespace visitors {

class DeclarationParseVisitor {
public:
  DeclarationParseVisitor(tokens::TokenStream &tokens,
                          core::ErrorReporter &errorReporter,
                          ExpressionParserVisitor &exprVisitor);

  // Main entry point
  nodes::DeclPtr parseDeclaration();

private:
  // Declaration type parsers
  nodes::DeclPtr parseVarDecl(bool isConst);
  nodes::DeclPtr parseFuncDecl();

  // Helper parsers
  nodes::AttributePtr parseAttribute();
  std::vector<nodes::AttributePtr> parseAttributeList();
  nodes::TypePtr parseType();
  nodes::TypePtr parsePrimaryType();
  nodes::TypePtr parseTypeModifiers(nodes::TypePtr baseType);
  nodes::ParamPtr parseParameter();
  std::vector<nodes::ParamPtr> parseParameterList();
  tokens::TokenType parseStorageClass();

  // Utility methods
  bool match(tokens::TokenType type);
  bool matchAny(const std::vector<tokens::TokenType> &types);
  bool check(tokens::TokenType type) const;
  void error(const std::string &message);
  void synchronize();
  
  // Member variables
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  ExpressionParserVisitor &exprVisitor_;
};

} // namespace visitors