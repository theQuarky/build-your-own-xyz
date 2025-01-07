#pragma once
#include "../lexer/token.h"
#include <memory>
#include <string>
#include <vector>

namespace ast {

// Forward declarations
class Expression;
class Statement;
class Type;

using ExprPtr = std::shared_ptr<Expression>;
using StmtPtr = std::shared_ptr<Statement>;
using TypePtr = std::shared_ptr<Type>;

// Base class for all AST nodes
class Node {
public:
  lexer::Token token;
  virtual ~Node() = default;
  Node(lexer::Token t) : token(std::move(t)) {}
};

// Base class for all expressions
class Expression : public Node {
public:
  using Node::Node;
};

// Literal values (numbers, strings, booleans)
class Literal : public Expression {
public:
  std::string value;

  Literal(lexer::Token t, std::string v)
      : Expression(std::move(t)), value(std::move(v)) {}
};

// Variable references
class Variable : public Expression {
public:
  std::string name;

  Variable(lexer::Token t) : Expression(std::move(t)), name(t.lexeme) {}
};

// Simple assignment
class Assignment : public Expression {
public:
  ExprPtr target;
  ExprPtr value;

  Assignment(lexer::Token t, ExprPtr tgt, ExprPtr val)
      : Expression(std::move(t)), target(std::move(tgt)),
        value(std::move(val)) {}
};

// Compound assignment (+=, -=, etc.)
class CompoundAssignment : public Expression {
public:
  ExprPtr target;
  ExprPtr value;
  std::string op;

  CompoundAssignment(lexer::Token t, ExprPtr tgt, ExprPtr val,
                     std::string operation)
      : Expression(std::move(t)), target(std::move(tgt)), value(std::move(val)),
        op(std::move(operation)) {}
};

// Increment/Decrement operations
class IncrementDecrement : public Expression {
public:
  ExprPtr operand;
  bool isPrefix;
  bool isIncrement;

  IncrementDecrement(lexer::Token t, ExprPtr op, bool prefix, bool increment)
      : Expression(std::move(t)), operand(std::move(op)), isPrefix(prefix),
        isIncrement(increment) {}
};

// Binary operations
class BinaryOp : public Expression {
public:
  ExprPtr left;
  ExprPtr right;

  BinaryOp(lexer::Token t, ExprPtr l, ExprPtr r)
      : Expression(std::move(t)), left(std::move(l)), right(std::move(r)) {}
};

// Unary operations
class UnaryOp : public Expression {
public:
  ExprPtr operand;

  UnaryOp(lexer::Token t, ExprPtr e)
      : Expression(std::move(t)), operand(std::move(e)) {}
};

// Function calls
class Call : public Expression {
public:
  ExprPtr callee;
  std::vector<ExprPtr> arguments;

  Call(lexer::Token t, ExprPtr c, std::vector<ExprPtr> args)
      : Expression(std::move(t)), callee(std::move(c)),
        arguments(std::move(args)) {}
};

// Base class for all statements
class Statement : public Node {
public:
  using Node::Node;
};

// Variable declarations
class VarDeclaration : public Statement {
public:
  std::string name;
  TypePtr type;
  ExprPtr initializer;
  bool isConst;

  VarDeclaration(lexer::Token t, std::string n, TypePtr ty, ExprPtr init,
                 bool c)
      : Statement(std::move(t)), name(std::move(n)), type(std::move(ty)),
        initializer(std::move(init)), isConst(c) {}
};

// Function declarations
class FunctionDeclaration : public Statement {
public:
  std::string name;
  std::vector<std::pair<std::string, TypePtr>> parameters;
  TypePtr returnType;
  std::vector<StmtPtr> body;

  FunctionDeclaration(lexer::Token t, std::string n,
                      std::vector<std::pair<std::string, TypePtr>> params,
                      TypePtr ret, std::vector<StmtPtr> b)
      : Statement(std::move(t)), name(std::move(n)),
        parameters(std::move(params)), returnType(std::move(ret)),
        body(std::move(b)) {}
};

// Return statements
class Return : public Statement {
public:
  ExprPtr value;

  Return(lexer::Token t, ExprPtr v)
      : Statement(std::move(t)), value(std::move(v)) {}
};

// If statements
class If : public Statement {
public:
  ExprPtr condition;
  StmtPtr thenBranch;
  StmtPtr elseBranch;

  If(lexer::Token t, ExprPtr cond, StmtPtr then, StmtPtr else_)
      : Statement(std::move(t)), condition(std::move(cond)),
        thenBranch(std::move(then)), elseBranch(std::move(else_)) {}
};

// Expression statements
class ExpressionStmt : public Statement {
public:
  ExprPtr expression;

  ExpressionStmt(lexer::Token t, ExprPtr e)
      : Statement(std::move(t)), expression(std::move(e)) {}
};

// Block of statements
class Block : public Statement {
public:
  std::vector<StmtPtr> statements;

  Block(lexer::Token t, std::vector<StmtPtr> stmts)
      : Statement(std::move(t)), statements(std::move(stmts)) {}
};

// Type definitions
class Type : public Node {
public:
  using Node::Node;
  virtual std::string getName() const = 0;
};

class BasicType : public Type {
public:
  std::string name;

  BasicType(lexer::Token t, std::string n)
      : Type(std::move(t)), name(std::move(n)) {}

  std::string getName() const override { return name; }
};

} // namespace ast