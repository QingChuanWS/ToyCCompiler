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
  ND_RETURN,      // return
  ND_EXPR_STMT,   // expression statement
  ND_VAR,         // variable
  ND_END,
};

class Node {
 public:
  Node(NodeKind kind = ND_END, Node* lhs = nullptr, Node* rhs = nullptr)
      : kind_(kind)
      , next_(nullptr)
      , lhs_(lhs)
      , rhs_(rhs)
      , val_(0)
      , var_() {}

  Node(long val)
      : kind_(ND_NUM)
      , next_(nullptr)
      , lhs_(nullptr)
      , rhs_(nullptr)
      , val_(val)
      , var_() {}

  Node(Var* var)
      : kind_(ND_VAR)
      , next_(nullptr)
      , lhs_(nullptr)
      , rhs_(nullptr)
      , val_(0)
      , var_(var) {}

  static void NodeListFree(Node* node);

 private:
  // post-order for AST delete
  static void NodeFree(Node* node);
  // parsing token list and generate AST.
  // program = node::stmt*
  static Node* Program(Token** tok);
  // stmt = "return" expr ";" | expr ";"
  static Node* Stmt(Token** tok);
  // expr = assign
  static Node* Expr(Token** tok);
  // assign = equality ("=" assign)?
  static Node* Assign(Token** tok);
  // equality = relational ("==" relational | "!=" relational)
  static Node* Equality(Token** tok);
  // relational = add ("<" add | "<=" add | ">" add | ">=" add)
  static Node* Relational(Token** tok);
  // add = mul ("+"mul | "-" mul)
  static Node* Add(Token** tok);
  // mul = unary ("*" unary | "/" unary)
  static Node* Mul(Token** tok);
  // unary = ("+" | "-") ? unary | primary
  static Node* Unary(Token** tok);
  // primary = "(" expr ")" | ident | num
  static Node* Primary(Token** tok);

  friend class CodeGenerator;
  friend class Function;

  NodeKind kind_;   // Node kind
  Node*    next_;   // next node (next AST)
  Node*    lhs_;    // left-head side
  Node*    rhs_;    // right-head side
  Var*     var_;    // use it if kind == ND_VAR
  long     val_;    // use it if Kind == ND_NUM
};

#endif   // !NODE_GRUAD
