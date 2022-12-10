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
#include "type.h"
#include "var.h"

#include <string>

enum NodeKind {
  ND_ADD,         // +
  ND_SUB,         // -
  ND_MUL,         // *
  ND_DIV,         // /
  ND_NEG,         // unary -
  ND_EQ,          // ==
  ND_NE,          // !=
  ND_LT,          // <
  ND_LE,          // <=
  ND_NUM,         // number
  ND_ASSIGN,      // =
  ND_ADDR,        // unary &
  ND_DEREF,       // *
  ND_EXPR_STMT,   // expression statement
  ND_RETURN,      // return
  ND_BLOCK,       // { ... }
  ND_IF,          // if
  ND_FOR,         // for and while
  ND_VAR,         // variable
  ND_END,
};

class Node {
 public:
  explicit Node(NodeKind kind, Token* tok)
      : kind_(kind)
      , tok_(tok)
      , next_(nullptr)
      , lhs_(nullptr)
      , rhs_(nullptr)
      , cond_(nullptr)
      , then_(nullptr)
      , els_(nullptr)
      , body_(nullptr)
      , init_(nullptr)
      , inc_(nullptr)
      , val_(0)
      , var_()
      , ty_(nullptr) {}

  explicit Node(NodeKind kind, Token* tok, Node* b_one)
      : kind_(kind)
      , tok_(tok)
      , next_(nullptr)
      , lhs_(nullptr)
      , rhs_(nullptr)
      , cond_(nullptr)
      , then_(nullptr)
      , els_(nullptr)
      , body_(nullptr)
      , init_(nullptr)
      , inc_(nullptr)
      , val_(0)
      , var_()
      , ty_(nullptr) {
    if (kind == ND_BLOCK) {
      body_ = b_one;
      return;
    }
    lhs_ = b_one;
  }

  explicit Node(NodeKind kind, Token* tok, Node* lhs, Node* rhs);

  explicit Node(long val, Token* tok)
      : kind_(ND_NUM)
      , tok_(tok)
      , next_(nullptr)
      , lhs_(nullptr)
      , rhs_(nullptr)
      , cond_(nullptr)
      , then_(nullptr)
      , els_(nullptr)
      , body_(nullptr)
      , init_(nullptr)
      , inc_(nullptr)
      , val_(val)
      , var_()
      , ty_(nullptr) {}

  explicit Node(Var* var, Token* tok)
      : kind_(ND_VAR)
      , tok_(tok)
      , next_(nullptr)
      , lhs_(nullptr)
      , rhs_(nullptr)
      , cond_(nullptr)
      , then_(nullptr)
      , els_(nullptr)
      , body_(nullptr)
      , init_(nullptr)
      , inc_(nullptr)
      , val_(0)
      , var_(var)
      , ty_(nullptr) {}

  static void NodeListFree(Node* node);

  // parsing token list and generate AST.
  // program = stmt*
  static Node* Program(Token* tok);

 private:
  // compound-stmt = stmt* "}"
  static Node* CompoundStmt(Token** rest, Token* tok);
  // declaration = declspec ( 
  //                 declarator ( "=" expr)? 
  //                 ("," declarator ("=" expr)? ) * )? ";"
  static Node* Declaration(Token** rest, Token* tok);
  // declspec = "int"
  static Type* Declspec(Token** rest,Token* tok);
  // declarator = "*"* ident
  static Type* Declarator(Token** rest, Token* tok, Type* ty);
  // stmt = "return" expr ";" |
  // "if" "(" expr ")" stmt ("else" stmt)? |
  // "for" "(" expr-stmt expr? ";" expr? ")" stmt |
  // "while" "(" expr ")" stmt |
  // "{" compuound-stmt |
  // expr-stmt
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
  // unary = ("+" | "-" | "*" | "&") ? unary | primary
  static Node* Unary(Token** rest, Token* tok);
  // primary = "(" expr ")" | ident | num
  static Node* Primary(Token** rest, Token* tok);

  // post-order for AST delete
  static void NodeFree(Node* node);

  // Report an error based on tok
  void ErrorTok(const char* fmt, ...);

  void TypeInfer();

  friend class CodeGenerator;
  friend class Function;

  NodeKind kind_;   // Node kind
  Token*   tok_;    // Representative node

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

  // for "for" statement
  Node* init_;
  Node* inc_;

  Var*  var_;   // use it if kind == ND_VAR
  long  val_;   // use it if Kind == ND_NUM
  Type* ty_;
};

#endif   // !NODE_GRUAD
