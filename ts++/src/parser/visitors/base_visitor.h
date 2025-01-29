/*****************************************************************************
 * File: base_visitor.h
 * Description: Main visitor interface that coordinates all visitor types
 *****************************************************************************/

#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/ast.h"
#include "parser/visitors/parse_visitor/base_parse_visitor.h"
#include "tokens/stream/token_stream.h"
#include <memory>

namespace visitors {

/**
 * @brief Main visitor interface that coordinates different visitor types
 *
 * This class acts as a facade for different visitor implementations,
 * providing a clean interface for the parser to use without needing
 * to know about specific visitor details.
 */
class BaseVisitor {
public:
  explicit BaseVisitor(tokens::TokenStream &tokens,
                       core::ErrorReporter &errorReporter)
      : tokens_(tokens), errorReporter_(errorReporter),
        parseVisitor_(
            std::make_unique<BaseParseVisitor>(tokens_, errorReporter_)) {}

  // Prevent copying but allow moving
  BaseVisitor(const BaseVisitor &) = delete;
  BaseVisitor &operator=(const BaseVisitor &) = delete;
  BaseVisitor(BaseVisitor &&) = default;
  BaseVisitor &operator=(BaseVisitor &&) = default;
  void addNode(nodes::NodePtr node) { ast_.addNode(std::move(node)); };
const parser::AST &getAST() {
    const auto &nodes = parseVisitor_->getNodes();
    for (const auto &node : nodes) {
      ast_.addNode(node); // Build AST from stored nodes
    }
    return ast_;
  };
  ~BaseVisitor() = default;

  /**
   * @brief Entry point for parsing phase
   * @return Success status of parsing
   */
  bool parse() { return parseVisitor_->visitParse(); }

  // Future visitor method stubs
  // bool typeCheck();
  // bool optimize();
  // bool generateCode();

private:
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  parser::AST ast_;
  // Different visitor implementations
  std::unique_ptr<BaseParseVisitor> parseVisitor_;
  // Future visitors:
  // std::unique_ptr<TypeCheckVisitor> typeCheckVisitor_;
  // std::unique_ptr<OptimizationVisitor> optimizationVisitor_;
  // std::unique_ptr<CodeGenVisitor> codeGenVisitor_;
};

} // namespace visitors