#include "type_check_visitor.h"
#include "parser/nodes/expression_nodes.h"
#include "tokens/token_type.h"
#include <iostream>

namespace visitors {

TypeCheckVisitor::TypeCheckVisitor(core::ErrorReporter &errorReporter)
    : errorReporter_(errorReporter) {

  // Initialize built-in types
  voidType_ = std::make_shared<ResolvedType>(ResolvedType::Void());
  intType_ = std::make_shared<ResolvedType>(ResolvedType::Int());
  floatType_ = std::make_shared<ResolvedType>(ResolvedType::Float());
  boolType_ = std::make_shared<ResolvedType>(ResolvedType::Bool());
  stringType_ = std::make_shared<ResolvedType>(ResolvedType::String());
  errorType_ = std::make_shared<ResolvedType>(ResolvedType::Error());

  // Initialize global scope
  currentScope_ = std::make_shared<TypeScope>();

  // Add built-in types to global scope
  currentScope_->declareType("void", voidType_);
  currentScope_->declareType("int", intType_);
  currentScope_->declareType("float", floatType_);
  currentScope_->declareType("bool", boolType_);
  currentScope_->declareType("string", stringType_);
}

bool TypeCheckVisitor::checkAST(const parser::AST &ast) {
  const auto &nodes = ast.getNodes();
  bool success = true;

  // First pass: collect all type declarations
  for (const auto &node : nodes) {
    if (auto classDecl =
            std::dynamic_pointer_cast<nodes::ClassDeclNode>(node)) {
      auto type = visitClassDecl(classDecl.get());
      currentScope_->declareType(classDecl->getName(), type);
    } else if (auto enumDecl =
                   std::dynamic_pointer_cast<nodes::EnumDeclNode>(node)) {
      auto type = visitEnumDecl(enumDecl.get());
      currentScope_->declareType(enumDecl->getName(), type);
    } else if (auto interfaceDecl =
                   std::dynamic_pointer_cast<nodes::InterfaceDeclNode>(node)) {
      auto type = visitInterfaceDecl(interfaceDecl.get());
      currentScope_->declareType(interfaceDecl->getName(), type);
    }
  }

  // Second pass: check all declarations and statements
  for (const auto &node : nodes) {
    if (auto varDecl = std::dynamic_pointer_cast<nodes::VarDeclNode>(node)) {
      auto type = visitVarDecl(varDecl.get());
      if (type->getKind() == ResolvedType::TypeKind::Error) {
        success = false;
      }
    } else if (auto funcDecl =
                   std::dynamic_pointer_cast<nodes::FunctionDeclNode>(node)) {
      auto type = visitFuncDecl(funcDecl.get());
      if (type->getKind() == ResolvedType::TypeKind::Error) {
        success = false;
      }
    } else if (auto stmt =
                   std::dynamic_pointer_cast<nodes::StatementNode>(node)) {
      auto type = visitStmt(stmt.get());
      if (type->getKind() == ResolvedType::TypeKind::Error) {
        success = false;
      }
    }
  }

  return success;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitVarDecl(const nodes::VarDeclNode *node) {
  // Check the initializer if present
  std::shared_ptr<ResolvedType> initType = nullptr;
  if (node->getInitializer()) {
    initType = visitExpr(node->getInitializer().get());
  }

  // Get the declared type if present
  std::shared_ptr<ResolvedType> declaredType = nullptr;
  if (node->getType()) {
    declaredType = visitType(node->getType().get());
  }

  // Determine the variable's type
  std::shared_ptr<ResolvedType> varType;
  if (declaredType) {
    varType = declaredType;

    // If there's an initializer, check compatibility
    if (initType) {
      if (!checkAssignmentCompatibility(declaredType, initType,
                                        node->getLocation())) {
        error(node->getLocation(),
              "Initializer type doesn't match variable type");
        return errorType_;
      }
    }
  } else if (initType) {
    // Type inference from initializer
    varType = initType;
  } else {
    error(node->getLocation(), "Variable declaration needs either a type or an "
                               "initializer for type inference");
    return errorType_;
  }

  // Add variable to current scope
  currentScope_->declareVariable(node->getName(), varType);

  return varType;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitFuncDecl(const nodes::FunctionDeclNode *node) {
  // Create a new scope for function body
  auto functionScope = currentScope_->createChildScope();
  std::swap(currentScope_, functionScope);

  // Process return type
  std::shared_ptr<ResolvedType> returnType;
  if (node->getReturnType()) {
    returnType = visitType(node->getReturnType().get());
  } else {
    returnType = voidType_; // Default to void if no return type specified
  }

  // Save return type for checking return statements
  currentFunctionReturnType_ = returnType;

  // Process parameters
  std::vector<std::shared_ptr<ResolvedType>> paramTypes;
  for (const auto &param : node->getParameters()) {
    auto paramType = visitParameter(param.get());
    paramTypes.push_back(paramType);

    // Add parameter to function scope
    currentScope_->declareVariable(param->getName(), paramType);
  }

  // Create function type
  auto functionType = std::make_shared<ResolvedType>(
      ResolvedType::Function(returnType, paramTypes));

  // Add function to parent scope
  functionScope->declareFunction(node->getName(), functionType);

  // Check function body
  if (node->getBody()) {
    visitBlock(node->getBody().get());
  }

  // Restore previous scope
  std::swap(currentScope_, functionScope);

  return functionType;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitClassDecl(const nodes::ClassDeclNode *node) {
  // Create a named type for the class
  auto classType =
      std::make_shared<ResolvedType>(ResolvedType::Named(node->getName()));

  // For now, we're just creating a placeholder type and not checking the class
  // body In a complete implementation, we would also process all class members
  // and inheritance

  return classType;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitEnumDecl(const nodes::EnumDeclNode *node) {
  // Create a named type for the enum
  auto enumType =
      std::make_shared<ResolvedType>(ResolvedType::Named(node->getName()));

  // For now, we're just creating a placeholder type
  // In a complete implementation, we would process all enum members

  return enumType;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitInterfaceDecl(const nodes::InterfaceDeclNode *node) {
  // Create a named type for the interface
  auto interfaceType =
      std::make_shared<ResolvedType>(ResolvedType::Named(node->getName()));

  // For now, we're just creating a placeholder type
  // In a complete implementation, we would process all interface methods

  return interfaceType;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitParameter(const nodes::ParameterNode *node) {
  // Get the parameter type
  auto paramType = visitType(node->getType().get());

  // Handle reference parameters
  if (node->isRef()) {
    paramType =
        std::make_shared<ResolvedType>(ResolvedType::Reference(paramType));
  }

  return paramType;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitStmt(const nodes::StatementNode *node) {
  if (auto exprStmt = dynamic_cast<const nodes::ExpressionStmtNode *>(node)) {
    return visitExprStmt(exprStmt);
  } else if (auto blockStmt = dynamic_cast<const nodes::BlockNode *>(node)) {
    return visitBlock(blockStmt);
  } else if (auto ifStmt = dynamic_cast<const nodes::IfStmtNode *>(node)) {
    return visitIfStmt(ifStmt);
  } else if (auto whileStmt =
                 dynamic_cast<const nodes::WhileStmtNode *>(node)) {
    return visitWhileStmt(whileStmt);
  } else if (auto forStmt = dynamic_cast<const nodes::ForStmtNode *>(node)) {
    return visitForStmt(forStmt);
  } else if (auto returnStmt =
                 dynamic_cast<const nodes::ReturnStmtNode *>(node)) {
    return visitReturnStmt(returnStmt);
  } else if (auto switchStmt =
                 dynamic_cast<const nodes::SwitchStmtNode *>(node)) {
    return visitSwitchStmt(switchStmt);
  }
  // ADD THIS NEW CASE:
  else if (auto declStmt =
               dynamic_cast<const nodes::DeclarationStmtNode *>(node)) {
    // Handle declaration statements by visiting the wrapped declaration
    auto decl = declStmt->getDeclaration();

    if (auto varDecl = std::dynamic_pointer_cast<nodes::VarDeclNode>(decl)) {
      return visitVarDecl(varDecl.get());
    } else if (auto funcDecl =
                   std::dynamic_pointer_cast<nodes::FunctionDeclNode>(decl)) {
      return visitFuncDecl(funcDecl.get());
    } else if (auto classDecl =
                   std::dynamic_pointer_cast<nodes::ClassDeclNode>(decl)) {
      return visitClassDecl(classDecl.get());
    } else if (auto enumDecl =
                   std::dynamic_pointer_cast<nodes::EnumDeclNode>(decl)) {
      return visitEnumDecl(enumDecl.get());
    } else if (auto interfaceDecl =
                   std::dynamic_pointer_cast<nodes::InterfaceDeclNode>(decl)) {
      return visitInterfaceDecl(interfaceDecl.get());
    } else {
      // For any other declaration types
      error(declStmt->getLocation(), "Unsupported declaration in statement");
      return errorType_;
    }
  } else {
    // Unhandled statement type
    error(node->getLocation(), "Unhandled statement type in type checking");
    return errorType_;
  }
}

// Add corresponding method:
std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitNewExpressionStmt(const nodes::NewExpressionNode *node) {
  // Implementation for the new statement type
  return voidType_;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitExprStmt(const nodes::ExpressionStmtNode *node) {
  // Expression statements just need to evaluate their expression
  visitExpr(node->getExpression().get());

  // Expression statements don't have a type themselves
  return voidType_;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitBlock(const nodes::BlockNode *node) {
  // Create a new scope for the block
  auto blockScope = currentScope_->createChildScope();
  std::swap(currentScope_, blockScope);

  // Process all statements in the block
  for (const auto &stmt : node->getStatements()) {
    visitStmt(stmt.get());
  }

  // Restore previous scope
  std::swap(currentScope_, blockScope);

  // Blocks don't have a type themselves
  return voidType_;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitIfStmt(const nodes::IfStmtNode *node) {
  // Check condition type (must be boolean)
  auto condType = visitExpr(node->getCondition().get());
  if (!condType->isImplicitlyConvertibleTo(*boolType_)) {
    error(node->getCondition()->getLocation(),
          "If condition must be convertible to boolean");
  }

  // Check then branch
  visitStmt(node->getThenBranch().get());

  // Check else branch if present
  if (node->getElseBranch()) {
    visitStmt(node->getElseBranch().get());
  }

  // If statements don't have a type themselves
  return voidType_;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitWhileStmt(const nodes::WhileStmtNode *node) {
  // Check condition type (must be boolean)
  auto condType = visitExpr(node->getCondition().get());
  if (!condType->isImplicitlyConvertibleTo(*boolType_)) {
    error(node->getCondition()->getLocation(),
          "While condition must be convertible to boolean");
  }

  // Check loop body
  visitStmt(node->getBody().get());

  // While statements don't have a type themselves
  return voidType_;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitForStmt(const nodes::ForStmtNode *node) {
  // Create a new scope for the for loop
  auto forScope = currentScope_->createChildScope();
  std::swap(currentScope_, forScope);

  // Check initializer if present
  if (node->getInitializer()) {
    visitStmt(node->getInitializer().get());
  }

  // Check condition if present (must be boolean)
  if (node->getCondition()) {
    auto condType = visitExpr(node->getCondition().get());
    if (!condType->isImplicitlyConvertibleTo(*boolType_)) {
      error(node->getCondition()->getLocation(),
            "For loop condition must be convertible to boolean");
    }
  }

  // Check increment if present
  if (node->getIncrement()) {
    visitExpr(node->getIncrement().get());
  }

  // Check loop body
  visitStmt(node->getBody().get());

  // Restore previous scope
  std::swap(currentScope_, forScope);

  // For statements don't have a type themselves
  return voidType_;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitReturnStmt(const nodes::ReturnStmtNode *node) {
  // Check the returned expression if present
  std::shared_ptr<ResolvedType> returnedType = voidType_;
  if (node->getValue()) {
    returnedType = visitExpr(node->getValue().get());
  }

  // Check compatibility with function return type
  if (!returnedType->isAssignableTo(*currentFunctionReturnType_)) {
    error(node->getLocation(),
          "Return value type doesn't match function return type");
  }

  // Return statements don't have a type themselves
  return voidType_;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitSwitchStmt(const nodes::SwitchStmtNode *node) {
  // Check switch expression
  auto exprType = visitExpr(node->getExpression().get());

  // Check each case
  for (const auto &switchCase : node->getCases()) {
    // Check case value if not default
    if (!switchCase.isDefault && switchCase.value) {
      auto caseType = visitExpr(switchCase.value.get());

      // Check if case value type is compatible with switch expression type
      if (!caseType->isAssignableTo(*exprType)) {
        error(switchCase.value->getLocation(),
              "Case value type doesn't match switch expression type");
      }
    }

    // Create a new scope for the case body
    auto caseScope = currentScope_->createChildScope();
    std::swap(currentScope_, caseScope);

    // Check all statements in the case body
    for (const auto &stmt : switchCase.body) {
      visitStmt(stmt.get());
    }

    // Restore previous scope
    std::swap(currentScope_, caseScope);
  }

  // Switch statements don't have a type themselves
  return voidType_;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitExpr(const nodes::ExpressionNode *node) {
  if (auto binary = dynamic_cast<const nodes::BinaryExpressionNode *>(node)) {
    return visitBinaryExpr(binary);
  } else if (auto unary =
                 dynamic_cast<const nodes::UnaryExpressionNode *>(node)) {
    return visitUnaryExpr(unary);
  } else if (auto literal =
                 dynamic_cast<const nodes::LiteralExpressionNode *>(node)) {
    return visitLiteralExpr(literal);
  } else if (auto identifier =
                 dynamic_cast<const nodes::IdentifierExpressionNode *>(node)) {
    return visitIdentifierExpr(identifier);
  } else if (auto call =
                 dynamic_cast<const nodes::CallExpressionNode *>(node)) {
    return visitCallExpr(call);
  } else if (auto assignment =
                 dynamic_cast<const nodes::AssignmentExpressionNode *>(node)) {
    return visitAssignmentExpr(assignment);
  } else if (auto member =
                 dynamic_cast<const nodes::MemberExpressionNode *>(node)) {
    return visitMemberExpr(member);
  } else if (auto index =
                 dynamic_cast<const nodes::IndexExpressionNode *>(node)) {
    return visitIndexExpr(index);
  } else if (auto newExpr =
                 dynamic_cast<const nodes::NewExpressionNode *>(node)) {
    return visitNewExpr(newExpr);
  } else if (auto cast =
                 dynamic_cast<const nodes::CastExpressionNode *>(node)) {
    return visitCastExpr(cast);
  } else if (auto array = dynamic_cast<const nodes::ArrayLiteralNode *>(node)) {
    return visitArrayLiteral(array);
  } else {
    // Unhandled expression type
    error(node->getLocation(), "Unhandled expression type in type checking");
    return errorType_;
  }
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitBinaryExpr(const nodes::BinaryExpressionNode *node) {
  auto leftType = visitExpr(node->getLeft().get());
  auto rightType = visitExpr(node->getRight().get());

  return checkBinaryOp(node->getExpressionType(), leftType, rightType,
                       node->getLocation());
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitUnaryExpr(const nodes::UnaryExpressionNode *node) {
  auto operandType = visitExpr(node->getOperand().get());

  return checkUnaryOp(node->getExpressionType(), operandType, node->isPrefix(),
                      node->getLocation());
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitLiteralExpr(const nodes::LiteralExpressionNode *node) {
  // Determine type based on the kind of literal
  switch (node->getExpressionType()) {
  case tokens::TokenType::NUMBER:
    // Check if there's a decimal point to determine if it's an int or float
    if (node->getValue().find('.') != std::string::npos) {
      return floatType_;
    } else {
      return intType_;
    }
  case tokens::TokenType::STRING_LITERAL:
    return stringType_;
  case tokens::TokenType::TRUE:
  case tokens::TokenType::FALSE:
    return boolType_;
  default:
    error(node->getLocation(), "Unknown literal type");
    return errorType_;
  }
}

std::shared_ptr<ResolvedType> TypeCheckVisitor::visitIdentifierExpr(
    const nodes::IdentifierExpressionNode *node) {
  // Look up the identifier in the current scope
  auto varType = currentScope_->lookupVariable(node->getName());

  if (!varType) {
    // If not found as a variable, try function lookup
    varType = currentScope_->lookupFunction(node->getName());
  }

  if (!varType) {
    error(node->getLocation(), "Undefined identifier: " + node->getName());
    return errorType_;
  }

  return varType;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitCallExpr(const nodes::CallExpressionNode *node) {
  // Check callee and get its type
  auto calleeType = visitExpr(node->getCallee().get());

  // Check if callee is a function type
  if (calleeType->getKind() != ResolvedType::TypeKind::Function) {
    error(node->getCallee()->getLocation(), "Cannot call non-function type");
    return errorType_;
  }

  // Check argument count
  const auto &paramTypes = calleeType->getParameterTypes();
  const auto &args = node->getArguments();

  if (paramTypes.size() != args.size()) {
    error(node->getLocation(), "Wrong number of arguments");
    return errorType_;
  }

  // Check each argument type
  for (size_t i = 0; i < args.size(); i++) {
    auto argType = visitExpr(args[i].get());

    if (!argType->isAssignableTo(*paramTypes[i])) {
      error(args[i]->getLocation(), "Argument type mismatch");
    }
  }

  // Return type is the function's return type
  return calleeType->getReturnType();
}

std::shared_ptr<ResolvedType> TypeCheckVisitor::visitAssignmentExpr(
    const nodes::AssignmentExpressionNode *node) {
  auto targetType = visitExpr(node->getTarget().get());
  auto valueType = visitExpr(node->getValue().get());

  auto op = node->getExpressionType();

  if (op == tokens::TokenType::EQUALS) {
    // Simple assignment
    if (!checkAssignmentCompatibility(targetType, valueType,
                                      node->getLocation())) {
      error(node->getLocation(), "Cannot assign incompatible type");
      return errorType_;
    }
  } else {
    // Compound assignment (+=, -=, etc.)
    // Convert to equivalent binary operation and check
    tokens::TokenType binaryOp;

    switch (op) {
    case tokens::TokenType::PLUS_EQUALS:
      binaryOp = tokens::TokenType::PLUS;
      break;
    case tokens::TokenType::MINUS_EQUALS:
      binaryOp = tokens::TokenType::MINUS;
      break;
    case tokens::TokenType::STAR_EQUALS:
      binaryOp = tokens::TokenType::STAR;
      break;
    case tokens::TokenType::SLASH_EQUALS:
      binaryOp = tokens::TokenType::SLASH;
      break;
    case tokens::TokenType::PERCENT_EQUALS:
      binaryOp = tokens::TokenType::PERCENT;
      break;
    default:
      error(node->getLocation(), "Unsupported compound assignment operator");
      return errorType_;
    }

    auto resultType =
        checkBinaryOp(binaryOp, targetType, valueType, node->getLocation());

    if (!resultType->isAssignableTo(*targetType)) {
      error(node->getLocation(),
            "Result of compound assignment is not assignable to target");
      return errorType_;
    }
  }

  // Assignment expressions have the type of the target
  return targetType;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitMemberExpr(const nodes::MemberExpressionNode *node) {
  auto objectType = visitExpr(node->getObject().get());

  // For now, we just return errorType since we don't have a way to look up
  // members In a complete implementation, we would check the object type and
  // look up the member

  error(node->getLocation(), "Member access type checking not implemented");
  return errorType_;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitIndexExpr(const nodes::IndexExpressionNode *node) {
  auto arrayType = visitExpr(node->getArray().get());
  auto indexType = visitExpr(node->getIndex().get());

  // Check that we're indexing an array
  if (arrayType->getKind() != ResolvedType::TypeKind::Array) {
    error(node->getArray()->getLocation(), "Cannot index non-array type");
    return errorType_;
  }

  // Check that the index is an integer
  if (!indexType->isAssignableTo(*intType_)) {
    error(node->getIndex()->getLocation(), "Array index must be an integer");
  }

  // Return the element type of the array
  return arrayType->getElementType();
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitNewExpr(const nodes::NewExpressionNode *node) {
  // In a complete implementation, we would look up the class type
  // and check constructor arguments

  auto classType = currentScope_->lookupType(node->getClassName());
  if (!classType) {
    error(node->getLocation(), "Undefined class: " + node->getClassName());
    return errorType_;
  }

  // Check constructor arguments
  // (not implemented here, would require looking up constructors)

  return classType;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitCastExpr(const nodes::CastExpressionNode *node) {
  auto exprType = visitExpr(node->getExpression().get());

  // Look up target type
  auto targetType = currentScope_->lookupType(node->getTargetType());
  if (!targetType) {
    error(node->getLocation(), "Undefined type: " + node->getTargetType());
    return errorType_;
  }

  // Check if cast is valid
  if (!exprType->isExplicitlyConvertibleTo(*targetType)) {
    error(node->getLocation(), "Invalid cast");
    return errorType_;
  }

  return targetType;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitArrayLiteral(const nodes::ArrayLiteralNode *node) {
  const auto &elements = node->getElements();

  if (elements.empty()) {
    // Empty array - we don't know the element type, so return error
    error(node->getLocation(), "Cannot determine type of empty array literal");
    return errorType_;
  }

  // Check first element to determine element type
  auto elementType = visitExpr(elements[0].get());

  // Check that all other elements have compatible types
  for (size_t i = 1; i < elements.size(); i++) {
    auto nextType = visitExpr(elements[i].get());

    if (!nextType->isAssignableTo(*elementType)) {
      error(elements[i]->getLocation(),
            "Array elements must have compatible types");
      return errorType_;
    }
  }

  // Create array type
  return std::make_shared<ResolvedType>(ResolvedType::Array(elementType));
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitType(const nodes::TypeNode *node) {
  if (auto primitive = dynamic_cast<const nodes::PrimitiveTypeNode *>(node)) {
    return visitPrimitiveType(primitive);
  } else if (auto named = dynamic_cast<const nodes::NamedTypeNode *>(node)) {
    return visitNamedType(named);
  } else if (auto array = dynamic_cast<const nodes::ArrayTypeNode *>(node)) {
    return visitArrayType(array);
  } else if (auto pointer =
                 dynamic_cast<const nodes::PointerTypeNode *>(node)) {
    return visitPointerType(pointer);
  } else if (auto function =
                 dynamic_cast<const nodes::FunctionTypeNode *>(node)) {
    return visitFunctionType(function);
  } else if (auto unionType =
                 dynamic_cast<const nodes::UnionTypeNode *>(node)) {
    return visitUnionType(unionType);
  } else if (auto smartPointer =
                 dynamic_cast<const nodes::SmartPointerTypeNode *>(node)) {
    return visitSmartPointerType(smartPointer);
  } else if (auto templateType =
                 dynamic_cast<const nodes::TemplateTypeNode *>(node)) {
    return visitTemplateType(templateType);
  } else {
    // Unhandled type
    error(node->getLocation(), "Unhandled type in type checking");
    return errorType_;
  }
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitPrimitiveType(const nodes::PrimitiveTypeNode *node) {
  switch (node->getType()) {
  case tokens::TokenType::VOID:
    return voidType_;
  case tokens::TokenType::INT:
    return intType_;
  case tokens::TokenType::FLOAT:
    return floatType_;
  case tokens::TokenType::BOOLEAN:
    return boolType_;
  case tokens::TokenType::STRING:
    return stringType_;
  default:
    error(node->getLocation(), "Unknown primitive type");
    return errorType_;
  }
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitNamedType(const nodes::NamedTypeNode *node) {
  // Look up the type by name
  auto type = currentScope_->lookupType(node->getName());

  if (!type) {
    error(node->getLocation(), "Undefined type: " + node->getName());
    return errorType_;
  }

  return type;
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitArrayType(const nodes::ArrayTypeNode *node) {
  auto elementType = visitType(node->getElementType().get());

  // Check size expression if present
  if (node->getSize()) {
    auto sizeType = visitExpr(node->getSize().get());

    if (!sizeType->isAssignableTo(*intType_)) {
      error(node->getSize()->getLocation(), "Array size must be an integer");
    }
  }

  return std::make_shared<ResolvedType>(ResolvedType::Array(elementType));
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitPointerType(const nodes::PointerTypeNode *node) {
  auto pointeeType = visitType(node->getBaseType().get());

  bool isUnsafe =
      node->getKind() == nodes::PointerTypeNode::PointerKind::Unsafe;

  return std::make_shared<ResolvedType>(
      ResolvedType::Pointer(pointeeType, isUnsafe));
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitFunctionType(const nodes::FunctionTypeNode *node) {
  auto returnType = visitType(node->getReturnType().get());

  std::vector<std::shared_ptr<ResolvedType>> paramTypes;
  for (const auto &paramType : node->getParameterTypes()) {
    paramTypes.push_back(visitType(paramType.get()));
  }

  return std::make_shared<ResolvedType>(
      ResolvedType::Function(returnType, paramTypes));
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitUnionType(const nodes::UnionTypeNode *node) {
  auto leftType = visitType(node->getLeft().get());
  auto rightType = visitType(node->getRight().get());

  return std::make_shared<ResolvedType>(
      ResolvedType::Union(leftType, rightType));
}

std::shared_ptr<ResolvedType> TypeCheckVisitor::visitSmartPointerType(
    const nodes::SmartPointerTypeNode *node) {
  auto pointeeType = visitType(node->getPointeeType().get());

  ResolvedType::SmartKind kind;
  switch (node->getKind()) {
  case nodes::SmartPointerTypeNode::SmartPointerKind::Shared:
    kind = ResolvedType::SmartKind::Shared;
    break;
  case nodes::SmartPointerTypeNode::SmartPointerKind::Unique:
    kind = ResolvedType::SmartKind::Unique;
    break;
  case nodes::SmartPointerTypeNode::SmartPointerKind::Weak:
    kind = ResolvedType::SmartKind::Weak;
    break;
  default:
    error(node->getLocation(), "Unknown smart pointer kind");
    return errorType_;
  }

  return std::make_shared<ResolvedType>(ResolvedType::Smart(pointeeType, kind));
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::visitTemplateType(const nodes::TemplateTypeNode *node) {
  auto baseType = visitType(node->getBaseType().get());

  std::vector<std::shared_ptr<ResolvedType>> argTypes;
  for (const auto &argType : node->getArguments()) {
    argTypes.push_back(visitType(argType.get()));
  }

  if (baseType->getKind() != ResolvedType::TypeKind::Named) {
    error(node->getBaseType()->getLocation(),
          "Template base type must be a named type");
    return errorType_;
  }

  return std::make_shared<ResolvedType>(
      ResolvedType::Template(baseType->getName(), argTypes));
}

void TypeCheckVisitor::error(const core::SourceLocation &location,
                             const std::string &message) {
  errorReporter_.error(location, message);
}

std::shared_ptr<ResolvedType>
TypeCheckVisitor::checkBinaryOp(tokens::TokenType op,
                                std::shared_ptr<ResolvedType> leftType,
                                std::shared_ptr<ResolvedType> rightType,
                                const core::SourceLocation &location) {

  // Handle arithmetic operators
  if (tokens::isArithmeticOperator(op)) {
    // Both operands must be numeric
    if ((leftType->getKind() == ResolvedType::TypeKind::Int ||
         leftType->getKind() == ResolvedType::TypeKind::Float) &&
        (rightType->getKind() == ResolvedType::TypeKind::Int ||
         rightType->getKind() == ResolvedType::TypeKind::Float)) {

      // If either operand is float, result is float
      if (leftType->getKind() == ResolvedType::TypeKind::Float ||
          rightType->getKind() == ResolvedType::TypeKind::Float) {
        return floatType_;
      } else {
        return intType_;
      }
    }

    // String concatenation with + operator
    if (op == tokens::TokenType::PLUS &&
        (leftType->getKind() == ResolvedType::TypeKind::String ||
         rightType->getKind() == ResolvedType::TypeKind::String)) {
      return stringType_;
    }

    error(location, "Invalid operands for arithmetic operator");
    return errorType_;
  }

  // Handle comparison operators
  if (tokens::isComparisonOperator(op)) {
    // Check if types are comparable
    if (leftType->isAssignableTo(*rightType) ||
        rightType->isAssignableTo(*leftType)) {
      return boolType_; // Comparison operators return boolean
    }

    error(location, "Cannot compare incompatible types");
    return errorType_;
  }

  // Handle logical operators
  if (tokens::isLogicalOperator(op)) {
    // Both operands must be convertible to boolean
    if (leftType->isImplicitlyConvertibleTo(*boolType_) &&
        rightType->isImplicitlyConvertibleTo(*boolType_)) {
      return boolType_;
    }

    error(location, "Logical operators require boolean operands");
    return errorType_;
  }

  // Handle bitwise operators
  if (tokens::isBitwiseOperator(op)) {
    // Both operands must be integers
    if (leftType->getKind() == ResolvedType::TypeKind::Int &&
        rightType->getKind() == ResolvedType::TypeKind::Int) {
      return intType_;
    }

    error(location, "Bitwise operators require integer operands");
    return errorType_;
  }

  // Unhandled operator
  error(location, "Unhandled binary operator in type checking");
  return errorType_;
}

std::shared_ptr<ResolvedType> TypeCheckVisitor::checkUnaryOp(
    tokens::TokenType op, std::shared_ptr<ResolvedType> operandType,
    bool isPrefix, const core::SourceLocation &location) {

  switch (op) {
  case tokens::TokenType::PLUS:
  case tokens::TokenType::MINUS:
    // Numeric unary operators require numeric operand
    if (operandType->getKind() == ResolvedType::TypeKind::Int ||
        operandType->getKind() == ResolvedType::TypeKind::Float) {
      return operandType; // Result has same type as operand
    }

    error(location, "Unary +/- requires numeric operand");
    return errorType_;

  case tokens::TokenType::EXCLAIM: // Logical NOT
    // Operand must be convertible to boolean
    if (operandType->isImplicitlyConvertibleTo(*boolType_)) {
      return boolType_;
    }

    error(location, "Logical NOT requires boolean operand");
    return errorType_;

  case tokens::TokenType::TILDE: // Bitwise NOT
    // Operand must be integer
    if (operandType->getKind() == ResolvedType::TypeKind::Int) {
      return intType_;
    }

    error(location, "Bitwise NOT requires integer operand");
    return errorType_;

  case tokens::TokenType::PLUS_PLUS:
  case tokens::TokenType::MINUS_MINUS:
    // Increment/decrement operators require numeric operand
    if (operandType->getKind() == ResolvedType::TypeKind::Int ||
        operandType->getKind() == ResolvedType::TypeKind::Float) {
      return operandType; // Result has same type as operand
    }

    error(location, "Increment/decrement requires numeric operand");
    return errorType_;

  default:
    error(location, "Unhandled unary operator in type checking");
    return errorType_;
  }
}

bool TypeCheckVisitor::checkAssignmentCompatibility(
    std::shared_ptr<ResolvedType> targetType,
    std::shared_ptr<ResolvedType> valueType,
    const core::SourceLocation &location) {

  if (valueType->isAssignableTo(*targetType)) {
    return true;
  }

  error(location, "Cannot assign " + valueType->toString() + " to " +
                      targetType->toString());
  return false;
}

} // namespace visitors