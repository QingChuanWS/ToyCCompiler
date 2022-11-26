/*
 * This project is exclusively owned by QingChuanWS and shall not be used for
 * commercial and profitting purpose without QingChuanWS's permission.
 *
 * @Author: bingshan45@163.com
 * Github: https://github.com/QingChuanWS
 * @Description:
 *
 * Copyright (c) 2022 by QingChuanWS, All Rights Reserved.
 */

#ifndef NODE_GRUAD
#define NODE_GRUAD

#include "token.h"
#include "tools.h"
#include "var.h"

#include <string>

enum NodeKind {
  ND_ADD,         // +
  ND_SUB,         // -
  ND_MUL,         // *
  ND_DIV,         // /
  ND_EQ,          // ==
  ND_NE,          // !=
  ND_LT,          // <
  ND_LE,          // <=
  ND_NUM,         // number
  ND_ASSIGN,      // =
  ND_EXPR_STMT,   // expression statement
  ND_RETURN,      // return
  ND_BLOCK,       // { ... }
  ND_IF,          // if
  ND_VAR,         // variable
  ND_END,
};

class Node {
 public:
  explicit Node(NodeKind kind)
      : kind_(kind)
      , next_(nullptr)
      , lhs_(nullptr)
      , rhs_(nullptr)
      , cond_(nullptr)
      , then_(nullptr)
      , els_(nullptr)
      , body_(nullptr)
      , val_(0)
      , var_() {}

  explicit Node(NodeKind kind, Node* b_one)
      : kind_(kind)
      , next_(nullptr)
      , lhs_(nullptr)
      , rhs_(nullptr)
      , cond_(nullptr)
      , then_(nullptr)
      , els_(nullptr)
      , body_(nullptr)
      , val_(0)
      , var_() {
    if (kind == ND_BLOCK) {
      body_ = b_one;
      return;
    }
    lhs_ = b_one;
  }

  explicit Node(NodeKind kind, Node* lhs, Node* rhs)
      : kind_(kind)
      , next_(nullptr)
      , lhs_(lhs)
      , rhs_(rhs)
      , cond_(nullptr)
      , then_(nullptr)
      , els_(nullptr)
      , body_(nullptr)
      , val_(0)
      , var_() {}

  explicit Node(long val)
      : kind_(ND_NUM)
      , next_(nullptr)
      , lhs_(nullptr)
      , rhs_(nullptr)
      , cond_(nullptr)
      , then_(nullptr)
      , els_(nullptr)
      , body_(nullptr)
      , val_(val)
      , var_() {}

  explicit Node(Var* var)
      : kind_(ND_VAR)
      , next_(nullptr)
      , lhs_(nullptr)
      , rhs_(nullptr)
      , cond_(nullptr)
      , then_(nullptr)
      , els_(nullptr)
      , body_(nullptr)
      , val_(0)
      , var_(var) {}

  static void NodeListFree(Node* node);

 private:
  // post-order for AST delete
  static void NodeFree(Node* node);
  // parsing token list and generate AST.
  // program = node::stmt*
  static Node* Program(Token* tok);
  // compound-stmt = stmt* "}"
  static Node* CompoundStmt(Token** rest, Token* tok);
  // stmt = "return" expr ";" |
  // "if" "(" expr ")" stmt ("else" stmt)? |
  // "{" compuound-stmt |
  // ExprStmt
  static Node* Stmt(Token** rest, Token* tok);
  // expr-stmt = expr ";"
  static Node* ExprStmt(Token** rest, Token* tok);
  // expr = assign
  static Node* Expr(Token** rest, Token* tok);
  // assign = equality ("=" assign)?
  static Node* Assign(Token** rest, Token* tok);
  // equality = relational ("==" relational | "!=" relational)
  static Node* Equality(Token** rest, Token* tok);
  // relational = add ("<" add | "<=" add | ">" add | ">=" add)
  static Node* Relational(Token** rest, Token* tok);
  // add = mul ("+"mul | "-" mul)
  static Node* Add(Token** rest, Token* tok);
  // mul = unary ("*" unary | "/" unary)
  static Node* Mul(Token** rest, Token* tok);
  // unary = ("+" | "-") ? unary | primary
  static Node* Unary(Token** rest, Token* tok);
  // primary = "(" expr ")" | ident | num
  static Node* Primary(Token** rest, Token* tok);

  friend class CodeGenerator;
  friend class Function;

  NodeKind kind_;   // Node kind

  // for next node (next AST)
  Node* next_;

  // for operation +-/*
  Node* lhs_;   // left-head side
  Node* rhs_;   // right-head side

  // for block
  Node* body_;

  // for "if" statement
  Node* cond_;
  Node* then_;
  Node* els_;

  Var* var_;   // use it if kind == ND_VAR
  long val_;   // use it if Kind == ND_NUM
};

#endif   // !NODE_GRUAD
