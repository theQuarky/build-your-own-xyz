// base_visitor.h
#pragma once
#include "core/diagnostics/error_reporter.h"
#include "parser/ast.h"
#include "parser/visitors/parse_visitor/base/base_parse_visitor.h"
#include "tokens/stream/token_stream.h"
#include <memory>

namespace visitors {

/**
 * Main visitor interface that coordinates different visitor types.
 * Acts as a facade for different visitor implementations.
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
  ~BaseVisitor() = default;

  // Entry point for parsing phase
  bool parse() { return parseVisitor_->visitParse(); }

  // AST management
  void addNode(nodes::NodePtr node) { ast_.addNode(std::move(node)); }
  const parser::AST &getAST() {
    const auto &nodes = parseVisitor_->getNodes();
    for (const auto &node : nodes) {
      ast_.addNode(node);
    }
    return ast_;
  }

private:
  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  parser::AST ast_;
  std::unique_ptr<BaseParseVisitor> parseVisitor_;
};

} // namespace visitors
