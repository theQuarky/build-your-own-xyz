#pragma once
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/type_nodes.h"

namespace visitors {

class IDeclarationVisitor {
public:
  virtual ~IDeclarationVisitor() = default;
  virtual nodes::TypePtr parseType() = 0;
  virtual nodes::BlockPtr parseBlock() = 0;
  virtual nodes::DeclPtr parseDeclaration() = 0;
};

} // namespace visitors