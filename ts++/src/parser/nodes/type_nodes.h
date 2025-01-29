/*****************************************************************************
 * File: type_nodes.h
 * Description: AST node definitions for TSPP type system
 *****************************************************************************/

#pragma once
#include "base_node.h"
#include "core/common/common_types.h"
#include "tokens/token_type.h"
#include <memory>
#include <vector>

namespace nodes {

// Forward declarations
class ExpressionNode;
using ExpressionPtr = std::shared_ptr<ExpressionNode>;

/**
 * Base class for all type nodes in the AST
 */
class TypeNode : public BaseNode {
public:
  TypeNode(const core::SourceLocation &loc) : BaseNode(loc) {}
  virtual ~TypeNode() = default;

  // Type-specific methods
  virtual bool isVoid() const { return false; }
  virtual bool isPrimitive() const { return false; }
  virtual bool isPointer() const { return false; }
  virtual bool isArray() const { return false; }
  virtual bool isFunction() const { return false; }
  virtual bool isTemplate() const { return false; }
};

using TypePtr = std::shared_ptr<TypeNode>;

/**
 * Primitive type node (void, int, float, etc.)
 */
class PrimitiveTypeNode : public TypeNode {
public:
  PrimitiveTypeNode(tokens::TokenType type, const core::SourceLocation &loc)
      : TypeNode(loc), type_(type) {}

  tokens::TokenType getType() const { return type_; }
  bool isPrimitive() const override { return true; }
  bool isVoid() const override { return type_ == tokens::TokenType::VOID; }
  bool accept(interface::BaseInterface *visitor) override {
    return visitor->visitParse();
  };

private:
  tokens::TokenType type_; // VOID, INT, FLOAT, etc.
};

/**
 * Named type node (user-defined types, type parameters)
 */
class NamedTypeNode : public TypeNode {
public:
  NamedTypeNode(const std::string &name, const core::SourceLocation &loc)
      : TypeNode(loc), name_(name) {}

  const std::string &getName() const { return name_; }
  bool accept(interface::BaseInterface *visitor) override {
    return visitor->visitParse();
  };

private:
  std::string name_;
};

/**
 * Qualified type node (namespace.type)
 */
class QualifiedTypeNode : public TypeNode {
public:
  QualifiedTypeNode(std::vector<std::string> qualifiers,
                    const core::SourceLocation &loc)
      : TypeNode(loc), qualifiers_(std::move(qualifiers)) {}

  const std::vector<std::string> &getQualifiers() const { return qualifiers_; }
  bool accept(interface::BaseInterface *visitor) override {
    return visitor->visitParse();
  };

private:
  std::vector<std::string> qualifiers_;
};

/**
 * Array type node (T[])
 */
class ArrayTypeNode : public TypeNode {
public:
  ArrayTypeNode(TypePtr elementType, ExpressionPtr size,
                const core::SourceLocation &loc)
      : TypeNode(loc), elementType_(std::move(elementType)),
        size_(std::move(size)) {}

  TypePtr getElementType() const { return elementType_; }
  ExpressionPtr getSize() const { return size_; }
  bool isArray() const override { return true; }
  bool accept(interface::BaseInterface *visitor) override {
    return visitor->visitParse();
  };

private:
  TypePtr elementType_; // Type of array elements
  ExpressionPtr size_;  // Optional size expression
};

/**
 * Pointer type node (T@)
 */
class PointerTypeNode : public TypeNode {
public:
  enum class PointerKind {
    Raw,    // T@
    Safe,   // T@safe
    Unsafe, // T@unsafe
    Aligned // T@aligned(N)
  };

  PointerTypeNode(TypePtr baseType, PointerKind kind, ExpressionPtr alignment,
                  const core::SourceLocation &loc)
      : TypeNode(loc), baseType_(std::move(baseType)), kind_(kind),
        alignment_(std::move(alignment)) {}

  TypePtr getBaseType() const { return baseType_; }
  PointerKind getKind() const { return kind_; }
  ExpressionPtr getAlignment() const { return alignment_; }
  bool isPointer() const override { return true; }
  bool accept(interface::BaseInterface *visitor) override {
    return visitor->visitParse();
  };

private:
  TypePtr baseType_;        // Type being pointed to
  PointerKind kind_;        // Kind of pointer
  ExpressionPtr alignment_; // Alignment for aligned pointers
};

/**
 * Reference type node (T&)
 */
class ReferenceTypeNode : public TypeNode {
public:
  ReferenceTypeNode(TypePtr baseType, const core::SourceLocation &loc)
      : TypeNode(loc), baseType_(std::move(baseType)) {}

  TypePtr getBaseType() const { return baseType_; }
  bool accept(interface::BaseInterface *visitor) override {
    return visitor->visitParse();
  };

private:
  TypePtr baseType_;
};

/**
 * Function type node (returns and parameters)
 */
class FunctionTypeNode : public TypeNode {
public:
  FunctionTypeNode(TypePtr returnType, std::vector<TypePtr> paramTypes,
                   const core::SourceLocation &loc)
      : TypeNode(loc), returnType_(std::move(returnType)),
        parameterTypes_(std::move(paramTypes)) {}

  TypePtr getReturnType() const { return returnType_; }
  const std::vector<TypePtr> &getParameterTypes() const {
    return parameterTypes_;
  }
  bool isFunction() const override { return true; }
  bool accept(interface::BaseInterface *visitor) override {
    return visitor->visitParse();
  };

private:
  TypePtr returnType_;
  std::vector<TypePtr> parameterTypes_;
};

/**
 * Template type node (Container<T>)
 */
class TemplateTypeNode : public TypeNode {
public:
  TemplateTypeNode(TypePtr baseType, std::vector<TypePtr> arguments,
                   const core::SourceLocation &loc)
      : TypeNode(loc), baseType_(std::move(baseType)),
        arguments_(std::move(arguments)) {}

  TypePtr getBaseType() const { return baseType_; }
  const std::vector<TypePtr> &getArguments() const { return arguments_; }
  bool isTemplate() const override { return true; }
  bool accept(interface::BaseInterface *visitor) override {
    return visitor->visitParse();
  };

private:
  TypePtr baseType_;               // Template type being instantiated
  std::vector<TypePtr> arguments_; // Template arguments
};

/**
 * Smart pointer type node (#shared<T>, #unique<T>, #weak<T>)
 */
class SmartPointerTypeNode : public TypeNode {
public:
  enum class SmartPointerKind { Shared, Unique, Weak };

  SmartPointerTypeNode(TypePtr pointeeType, SmartPointerKind kind,
                       const core::SourceLocation &loc)
      : TypeNode(loc), pointeeType_(std::move(pointeeType)), kind_(kind) {}

  TypePtr getPointeeType() const { return pointeeType_; }
  SmartPointerKind getKind() const { return kind_; }
  bool accept(interface::BaseInterface *visitor) override {
    return visitor->visitParse();
  };

private:
  TypePtr pointeeType_;
  SmartPointerKind kind_;
};

// Forward declarations for type visitors
class TypeVisitor {
public:
  virtual ~TypeVisitor() = default;
  virtual bool visitPrimitiveType(PrimitiveTypeNode *node) = 0;
  virtual bool visitNamedType(NamedTypeNode *node) = 0;
  virtual bool visitQualifiedType(QualifiedTypeNode *node) = 0;
  virtual bool visitArrayType(ArrayTypeNode *node) = 0;
  virtual bool visitPointerType(PointerTypeNode *node) = 0;
  virtual bool visitReferenceType(ReferenceTypeNode *node) = 0;
  virtual bool visitFunctionType(FunctionTypeNode *node) = 0;
  virtual bool visitTemplateType(TemplateTypeNode *node) = 0;
  virtual bool visitSmartPointerType(SmartPointerTypeNode *node) = 0;
};

} // namespace nodes