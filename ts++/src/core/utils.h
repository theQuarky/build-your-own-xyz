#include "../lexer/lexer.h"
#include "../parser/ast.h"

void printTokens(const std::vector<lexer::Token> &tokens);
void printAST(const std::vector<ast::StmtPtr> &statements, int indent = 0);