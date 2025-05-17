#pragma once
#include "core/diagnostics/error_reporter.h"
#include "llvm_context.h"
#include "llvm_function.h"
#include "llvm_optimizer.h"
#include "llvm_type_builder.h"
#include "llvm_value.h"
#include "parser/ast.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/nodes/statement_nodes.h"
#include "parser/nodes/type_nodes.h"
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>

namespace codegen {

/**
 * @class LLVMCodeGen
 * @brief Main LLVM code generator for TS++
 *
 * This class is responsible for traversing the AST and generating LLVM IR code.
 */
class LLVMCodeGen {
public:
  /**
   * @brief Constructs a code generator
   * @param errorReporter Error reporter for diagnostics
   * @param moduleName Name for the LLVM module
   */
  LLVMCodeGen(core::ErrorReporter &errorReporter,
              const std::string &moduleName = "tspp_module");

  /**
   * @brief Generates code for an AST
   * @param ast The TS++ AST
   * @return True if code generation was successful
   */
  bool generateCode(const parser::AST &ast);

  /**
   * @brief Optimizes the generated code
   * @param level Optimization level
   */
  void optimize(OptimizationLevel level = OptimizationLevel::O2);

  /**
   * @brief Writes the generated code to a file
   * @param filename Path to the output file
   * @return True if the operation was successful
   */
  bool writeToFile(const std::string &filename) const;

  /**
   * @brief Gets the LLVM context
   * @return Reference to the LLVM context
   */
  LLVMContext &getContext() { return context_; }

private:
  // Type declaration prepass
  void declareTypes(const parser::AST &ast);

  // Declaration visitors
  void visitGlobalDecl(const nodes::NodePtr &node);
  LLVMValue visitVarDecl(const nodes::VarDeclNode *node, bool isGlobal = false);
  llvm::Function *visitFuncDecl(const nodes::FunctionDeclNode *node);
  void visitClassDecl(const nodes::ClassDeclNode *node);
  void visitNamespaceDecl(const nodes::NamespaceDeclNode *node);
  void visitEnumDecl(const nodes::EnumDeclNode *node);
  void visitInterfaceDecl(const nodes::InterfaceDeclNode *node);
  LLVMValue visitParameter(const nodes::ParameterNode *node);

  // Statement visitors
  LLVMValue visitStmt(const nodes::StatementNode *node);
  LLVMValue visitExprStmt(const nodes::ExpressionStmtNode *node);
  LLVMValue visitBlock(const nodes::BlockNode *node);
  LLVMValue visitIfStmt(const nodes::IfStmtNode *node);
  LLVMValue visitWhileStmt(const nodes::WhileStmtNode *node);
  LLVMValue visitDoWhileStmt(const nodes::DoWhileStmtNode *node);
  LLVMValue visitForStmt(const nodes::ForStmtNode *node);
  LLVMValue visitForOfStmt(const nodes::ForOfStmtNode *node);
  LLVMValue visitReturnStmt(const nodes::ReturnStmtNode *node);
  LLVMValue visitBreakStmt(const nodes::BreakStmtNode *node);
  LLVMValue visitContinueStmt(const nodes::ContinueStmtNode *node);
  LLVMValue visitSwitchStmt(const nodes::SwitchStmtNode *node);
  LLVMValue visitTryStmt(const nodes::TryStmtNode *node);
  LLVMValue visitThrowStmt(const nodes::ThrowStmtNode *node);
  LLVMValue visitDeclStmt(const nodes::DeclarationStmtNode *node);

  // Expression visitors
  LLVMValue visitExpr(const nodes::ExpressionNode *node);
  LLVMValue visitBinaryExpr(const nodes::BinaryExpressionNode *node);
  LLVMValue visitUnaryExpr(const nodes::UnaryExpressionNode *node);
  LLVMValue visitLiteralExpr(const nodes::LiteralExpressionNode *node);
  LLVMValue visitIdentifierExpr(const nodes::IdentifierExpressionNode *node);
  LLVMValue visitCallExpr(const nodes::CallExpressionNode *node);
  LLVMValue visitMemberExpr(const nodes::MemberExpressionNode *node);
  LLVMValue visitIndexExpr(const nodes::IndexExpressionNode *node);
  LLVMValue visitAssignmentExpr(const nodes::AssignmentExpressionNode *node);
  LLVMValue visitNewExpr(const nodes::NewExpressionNode *node);
  LLVMValue visitCastExpr(const nodes::CastExpressionNode *node);
  LLVMValue visitArrayLiteral(const nodes::ArrayLiteralNode *node);

  // Helper methods
  void error(const core::SourceLocation &location, const std::string &message);
  void warning(const core::SourceLocation &location,
               const std::string &message);

  // Loop management for break/continue
  struct LoopInfo {
    llvm::BasicBlock *continueDest;
    llvm::BasicBlock *breakDest;
  };
  void pushLoop(llvm::BasicBlock *continueDest, llvm::BasicBlock *breakDest);
  void popLoop();
  LoopInfo *getCurrentLoop();

  // Core components
  core::ErrorReporter &errorReporter_;
  LLVMContext context_;
  LLVMTypeBuilder typeBuilder_;
  LLVMOptimizer optimizer_;

  // Function generation state
  std::unique_ptr<LLVMFunction> currentFunction_;
  std::stack<LoopInfo> loopStack_;

  // Namespace tracking
  std::vector<std::string> currentNamespace_;
  std::string getCurrentNamespacePrefix() const;

  // Function lookup table
  std::unordered_map<std::string, llvm::Function *> functionTable_;
};

} // namespace codegen