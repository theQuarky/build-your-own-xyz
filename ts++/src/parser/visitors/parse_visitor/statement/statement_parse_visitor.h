#pragma once
#include "branch_stmt_visitor.h"
#include "flowctr_stmt_visitor.h"
#include "loop_stmt_visitor.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/statement_nodes.h"
#include "parser/visitors/parse_visitor/declaration/ideclaration_visitor.h"
#include "parser/visitors/parse_visitor/statement/istatement_visitor.h"
#include "trycatch_stmt.visitor.h"
#include <cassert>
#include <iostream>

namespace visitors {

class StatementParseVisitor : public IStatementVisitor {
public:
  StatementParseVisitor(tokens::TokenStream &tokens,
                        core::ErrorReporter &errorReporter,
                        IExpressionVisitor &exprVisitor)
      : tokens_(tokens), errorReporter_(errorReporter),
        exprVisitor_(exprVisitor),
        branchVisitor_(tokens, errorReporter, exprVisitor, *this),
        loopVisitor_(tokens, errorReporter, exprVisitor, *this),
        flowVisitor_(tokens, errorReporter, exprVisitor),
        tryVisitor_(tokens, errorReporter, *this) {}

  void setDeclarationVisitor(IDeclarationVisitor *declVisitor) {
    declVisitor_ = declVisitor;
  }

  nodes::StmtPtr parseStatement() override {
    try {
      if (isDeclarationStart()) {
        return parseDeclarationStatement();
      }

      if (match(tokens::TokenType::LEFT_BRACE)) {
        return parseBlock();
      }
      if (match(tokens::TokenType::IF)) {
        return branchVisitor_.parseIfStatement();
      }
      if (match(tokens::TokenType::SWITCH)) {
        return branchVisitor_.parseSwitchStatement();
      }
      if (match(tokens::TokenType::WHILE)) {
        return loopVisitor_.parseWhileStatement();
      }
      if (match(tokens::TokenType::DO)) {
        return loopVisitor_.parseDoWhileStatement();
      }
      if (match(tokens::TokenType::FOR)) {
        return loopVisitor_.parseForStatement();
      }
      if (match(tokens::TokenType::TRY)) {
        return tryVisitor_.parseTryStatement();
      }
      if (match(tokens::TokenType::RETURN)) {
        return flowVisitor_.parseReturn();
      }
      if (match(tokens::TokenType::BREAK)) {
        return flowVisitor_.parseBreak();
      }
      if (match(tokens::TokenType::CONTINUE)) {
        return flowVisitor_.parseContinue();
      }
      if (match(tokens::TokenType::THROW)) {
        return flowVisitor_.parseThrow();
      }

      return parseExpressionStatement();
    } catch (const std::exception &e) {
      error(std::string("Error parsing statement: ") + e.what());
      synchronize();
      return nullptr;
    }
  }

  nodes::BlockPtr parseBlock() override {
    auto location = tokens_.previous().getLocation();
    std::vector<nodes::StmtPtr> statements;

    while (!check(tokens::TokenType::RIGHT_BRACE) && !tokens_.isAtEnd()) {

      if (auto stmt = parseStatement()) {
        statements.push_back(std::move(stmt));
      } else {
        // If parsing failed, print current token
        synchronize();
      }
    }

    if (!consume(tokens::TokenType::RIGHT_BRACE, "Expected '}' after block")) {
      return nullptr;
    }

    return std::make_shared<nodes::BlockNode>(std::move(statements), location);
  }

private:
  nodes::StmtPtr parseDeclarationStatement() {
    // Let the declaration visitor handle the declaration
    if (auto decl = declVisitor_->parseDeclaration()) {
      // Wrap it in a statement node
      return std::make_shared<nodes::DeclarationStmtNode>(std::move(decl),
                                                          decl->getLocation());
    }
    return nullptr;
  }

  bool isDeclarationStart() const {
    return check(tokens::TokenType::LET) || check(tokens::TokenType::CONST) ||
           check(tokens::TokenType::FUNCTION) ||
           check(tokens::TokenType::STACK) || check(tokens::TokenType::HEAP) ||
           check(tokens::TokenType::STATIC);
  }

  nodes::StmtPtr parseExpressionStatement();
  nodes::StmtPtr parseAssemblyStatement();

  // Add these utility methods if not already present
  inline bool match(tokens::TokenType type) {
    if (check(type)) {
      tokens_.advance();
      return true;
    }
    return false;
  }

  inline bool check(tokens::TokenType type) const {
    return !tokens_.isAtEnd() && tokens_.peek().getType() == type;
  }

  inline bool consume(tokens::TokenType type, const std::string &message) {
    if (check(type)) {
      tokens_.advance();
      return true;
    }
    error(message);
    return false;
  }

  inline void error(const std::string &message) {
    errorReporter_.error(tokens_.peek().getLocation(), message);
  }

  void synchronize() {
    tokens_.advance();

    while (!tokens_.isAtEnd()) {
      if (tokens_.previous().getType() == tokens::TokenType::SEMICOLON) {
        return;
      }

      switch (tokens_.peek().getType()) {
      case tokens::TokenType::CLASS:
      case tokens::TokenType::FUNCTION:
      case tokens::TokenType::LET:
      case tokens::TokenType::CONST:
      case tokens::TokenType::IF:
      case tokens::TokenType::WHILE:
      case tokens::TokenType::RETURN:
        return;
      default:
        tokens_.advance();
      }
    }
  }

  tokens::TokenStream &tokens_;
  core::ErrorReporter &errorReporter_;
  IExpressionVisitor &exprVisitor_;
  IDeclarationVisitor *declVisitor_ =
      nullptr; // Now a pointer, set after construction

  BranchStatementVisitor branchVisitor_;
  LoopStatementVisitor loopVisitor_;
  FlowControlVisitor flowVisitor_;
  TryCatchStatementVisitor tryVisitor_;
};

} // namespace visitors