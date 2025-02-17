/*****************************************************************************
 * File: declaration_nodes.h
 * Description: AST node definitions for declarations in TSPP language
 *****************************************************************************/

#pragma once
#include "base_node.h"
#include "expression_nodes.h"
#include "type_nodes.h"
#include <memory>
#include <vector>

namespace nodes {

// Forward declaration for BlockNode - will be implemented later
class BlockNode;
using BlockPtr = std::shared_ptr<BlockNode>;
using AttributePtr = std::shared_ptr<AttributeNode>;

/**
 * Base class for all declaration nodes
 */
class DeclarationNode : public BaseNode {
public:
  DeclarationNode(const std::string &name, const core::SourceLocation &loc)
      : BaseNode(loc), name_(name) {}

  virtual ~DeclarationNode() = default;

  const std::string &getName() const { return name_; }
  const std::vector<AttributePtr> &getAttributes() const { return attributes_; }
  void addAttribute(AttributePtr attr) {
    attributes_.push_back(std::move(attr));
  }

protected:
  std::string name_;                     // Declaration name
  std::vector<AttributePtr> attributes_; // Attributes (#stack, #inline, etc.)
};

using DeclPtr = std::shared_ptr<DeclarationNode>;

/**
 * Variable declaration node (let x: int = 42)
 */
class VarDeclNode : public DeclarationNode {
public:
  VarDeclNode(const std::string &name, TypePtr type, ExpressionPtr initializer,
              tokens::TokenType storageClass, bool isConst,
              const core::SourceLocation &loc)
      : DeclarationNode(name, loc), type_(std::move(type)),
        initializer_(std::move(initializer)), storageClass_(storageClass),
        isConst_(isConst) {}

  TypePtr getType() const { return type_; }
  ExpressionPtr getInitializer() const { return initializer_; }
  tokens::TokenType getStorageClass() const { return storageClass_; }
  bool isConst() const { return isConst_; }

  bool accept(interface::BaseInterface *visitor) override {
    return visitor->visitParse();
  }

private:
  TypePtr type_;                   // Variable type (can be nullptr if inferred)
  ExpressionPtr initializer_;      // Initial value (can be nullptr)
  tokens::TokenType storageClass_; // #stack, #heap, #static
  bool isConst_;                   // Whether declared with 'const'
};

/**
 * Function parameter declaration
 */
class ParameterNode : public DeclarationNode {
public:
  ParameterNode(const std::string &name, TypePtr type,
                ExpressionPtr defaultValue, bool isRef, bool isConst,
                const core::SourceLocation &loc)
      : DeclarationNode(name, loc), type_(std::move(type)),
        defaultValue_(std::move(defaultValue)), isRef_(isRef),
        isConst_(isConst) {}

  TypePtr getType() const { return type_; }
  ExpressionPtr getDefaultValue() const { return defaultValue_; }
  bool isRef() const { return isRef_; }
  bool isConst() const { return isConst_; }

  bool accept(interface::BaseInterface *visitor) override {
    return visitor->visitParse();
  }

private:
  TypePtr type_;               // Parameter type
  ExpressionPtr defaultValue_; // Default value (can be nullptr)
  bool isRef_;                 // Whether parameter is by reference
  bool isConst_;               // Whether parameter is const
};

using ParamPtr = std::shared_ptr<ParameterNode>;

/**
 * Function declaration node
 */
class FunctionDeclNode : public DeclarationNode {
public:
  FunctionDeclNode(const std::string &name, std::vector<ParamPtr> params,
                   TypePtr returnType,
                   BlockPtr body, // Can be nullptr for declarations
                   bool isAsync, const core::SourceLocation &loc)
      : DeclarationNode(name, loc), parameters_(std::move(params)),
        returnType_(std::move(returnType)), body_(std::move(body)),
        isAsync_(isAsync) {}

  const std::vector<ParamPtr> &getParameters() const { return parameters_; }
  TypePtr getReturnType() const { return returnType_; }
  BlockPtr getBody() const { return body_; }
  bool isAsync() const { return isAsync_; }

  bool accept(interface::BaseInterface *visitor) override {
    return visitor->visitParse();
  }

private:
  std::vector<ParamPtr> parameters_; // Function parameters
  TypePtr returnType_;               // Return type (can be nullptr for void)
  BlockPtr body_; // Function body (can be nullptr for declarations)
  bool isAsync_;  // Whether function is async
};

/**
 * Generic Function declaration node
 */
class GenericFunctionDeclNode : public FunctionDeclNode {
public:
  GenericFunctionDeclNode(
      const std::string &name, std::vector<TypePtr> genericParams,
      std::vector<ParamPtr> params, TypePtr returnType,
      std::vector<std::pair<std::string, TypePtr>> constraints, BlockPtr body,
      bool isAsync, const core::SourceLocation &loc)
      : FunctionDeclNode(name, std::move(params), std::move(returnType),
                         std::move(body), isAsync, loc),
        genericParams_(std::move(genericParams)),
        constraints_(std::move(constraints)) {}

  const std::vector<TypePtr> &getGenericParams() const {
    return genericParams_;
  }
  const std::vector<std::pair<std::string, TypePtr>> &getConstraints() const {
    return constraints_;
  }

  bool accept(interface::BaseInterface *visitor) override {
    return visitor->visitParse();
  }

private:
  std::vector<TypePtr>
      genericParams_; // The generic type parameters (T, U, etc)
  std::vector<std::pair<std::string, TypePtr>>
      constraints_; // The where constraints
};

// Forward declarations for declaration visitors
class DeclVisitor {
public:
  virtual ~DeclVisitor() = default;
  virtual bool visitVarDecl(VarDeclNode *node) = 0;
  virtual bool visitParameter(ParameterNode *node) = 0;
  virtual bool visitFunction(FunctionDeclNode *node) = 0;
  virtual bool visitAttribute(AttributeNode *node) = 0;
};

} // namespace nodes