#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/ast.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/nodes/statement_nodes.h"
#include "parser/nodes/type_nodes.h"
#include "resolved_type.h"
#include "type_scope.h"
#include <memory>

namespace visitors {

/**
 * @brief Type checker visitor that traverses the AST
 *
 * Traverses the AST and performs type checking on nodes, reporting errors
 * for type mismatches and other type errors.
 */
class TypeCheckVisitor {
public:
  explicit TypeCheckVisitor(core::ErrorReporter &errorReporter);

  // Entry point for type checking an AST
  bool checkAST(const parser::AST &ast);

  // Declarations
  std::shared_ptr<ResolvedType> visitVarDecl(const nodes::VarDeclNode *node);
  std::shared_ptr<ResolvedType>
  visitFuncDecl(const nodes::FunctionDeclNode *node);
  std::shared_ptr<ResolvedType>
  visitClassDecl(const nodes::ClassDeclNode *node);
  std::shared_ptr<ResolvedType> visitEnumDecl(const nodes::EnumDeclNode *node);
  std::shared_ptr<ResolvedType>
  visitParameter(const nodes::ParameterNode *node);
  std::shared_ptr<ResolvedType>
  visitInterfaceDecl(const nodes::InterfaceDeclNode *node);

  // Statements
  std::shared_ptr<ResolvedType> visitStmt(const nodes::StatementNode *node);
  std::shared_ptr<ResolvedType>
  visitExprStmt(const nodes::ExpressionStmtNode *node);
  std::shared_ptr<ResolvedType> visitBlock(const nodes::BlockNode *node);
  std::shared_ptr<ResolvedType> visitIfStmt(const nodes::IfStmtNode *node);
  std::shared_ptr<ResolvedType>
  visitWhileStmt(const nodes::WhileStmtNode *node);
  std::shared_ptr<ResolvedType> visitForStmt(const nodes::ForStmtNode *node);
  std::shared_ptr<ResolvedType>
  visitReturnStmt(const nodes::ReturnStmtNode *node);
  std::shared_ptr<ResolvedType>
  visitSwitchStmt(const nodes::SwitchStmtNode *node);

  // Expressions
  std::shared_ptr<ResolvedType> visitExpr(const nodes::ExpressionNode *node);
  std::shared_ptr<ResolvedType>
  visitBinaryExpr(const nodes::BinaryExpressionNode *node);
  std::shared_ptr<ResolvedType>
  visitUnaryExpr(const nodes::UnaryExpressionNode *node);
  std::shared_ptr<ResolvedType>
  visitLiteralExpr(const nodes::LiteralExpressionNode *node);
  std::shared_ptr<ResolvedType>
  visitIdentifierExpr(const nodes::IdentifierExpressionNode *node);
  std::shared_ptr<ResolvedType>
  visitCallExpr(const nodes::CallExpressionNode *node);
  std::shared_ptr<ResolvedType>
  visitAssignmentExpr(const nodes::AssignmentExpressionNode *node);
  std::shared_ptr<ResolvedType>
  visitMemberExpr(const nodes::MemberExpressionNode *node);
  std::shared_ptr<ResolvedType>
  visitIndexExpr(const nodes::IndexExpressionNode *node);
  std::shared_ptr<ResolvedType>
  visitNewExpr(const nodes::NewExpressionNode *node);
  std::shared_ptr<ResolvedType>
  visitCastExpr(const nodes::CastExpressionNode *node);
  std::shared_ptr<ResolvedType>
  visitArrayLiteral(const nodes::ArrayLiteralNode *node);

  // Types
  std::shared_ptr<ResolvedType> visitType(const nodes::TypeNode *node);
  std::shared_ptr<ResolvedType>
  visitPrimitiveType(const nodes::PrimitiveTypeNode *node);
  std::shared_ptr<ResolvedType>
  visitNamedType(const nodes::NamedTypeNode *node);
  std::shared_ptr<ResolvedType>
  visitArrayType(const nodes::ArrayTypeNode *node);
  std::shared_ptr<ResolvedType>
  visitPointerType(const nodes::PointerTypeNode *node);
  std::shared_ptr<ResolvedType>
  visitFunctionType(const nodes::FunctionTypeNode *node);
  std::shared_ptr<ResolvedType>
  visitUnionType(const nodes::UnionTypeNode *node);
  std::shared_ptr<ResolvedType>
  visitSmartPointerType(const nodes::SmartPointerTypeNode *node);
  std::shared_ptr<ResolvedType>
  visitTemplateType(const nodes::TemplateTypeNode *node);

private:
  // Report a type error
  void error(const core::SourceLocation &location, const std::string &message);

  // Helper for binary expression type checking
  std::shared_ptr<ResolvedType>
  checkBinaryOp(tokens::TokenType op, std::shared_ptr<ResolvedType> leftType,
                std::shared_ptr<ResolvedType> rightType,
                const core::SourceLocation &location);

  // Helper for unary expression type checking
  std::shared_ptr<ResolvedType>
  checkUnaryOp(tokens::TokenType op, std::shared_ptr<ResolvedType> operandType,
               bool isPrefix, const core::SourceLocation &location);

  // Helper for checking assignment compatibility
  bool checkAssignmentCompatibility(std::shared_ptr<ResolvedType> targetType,
                                    std::shared_ptr<ResolvedType> valueType,
                                    const core::SourceLocation &location);

  // Current type checking state
  core::ErrorReporter &errorReporter_;
  std::shared_ptr<TypeScope> currentScope_;
  std::shared_ptr<ResolvedType>
      currentFunctionReturnType_; // For return statement checking

  // Builtin type instances
  std::shared_ptr<ResolvedType> voidType_;
  std::shared_ptr<ResolvedType> intType_;
  std::shared_ptr<ResolvedType> floatType_;
  std::shared_ptr<ResolvedType> boolType_;
  std::shared_ptr<ResolvedType> stringType_;
  std::shared_ptr<ResolvedType> errorType_;
};

} // namespace visitors