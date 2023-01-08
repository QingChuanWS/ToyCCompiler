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

#include <string>

#include "object.h"
#include "token.h"
#include "tools.h"
#include "type.h"

enum NodeKind {
  ND_ADD,        // +
  ND_SUB,        // -
  ND_MUL,        // *
  ND_DIV,        // /
  ND_NEG,        // unary -
  ND_EQ,         // ==
  ND_NE,         // !=
  ND_LT,         // <
  ND_LE,         // <=
  ND_NUM,        // number
  ND_ASSIGN,     // =
  ND_ADDR,       // unary &
  ND_DEREF,      // *
  ND_EXPR_STMT,  // expression statement
  ND_RETURN,     // return
  ND_BLOCK,      // { ... }
  ND_CALL,       // Function call
  ND_IF,         // if
  ND_FOR,        // for and while
  ND_VAR,        // variable
  ND_END,
};

class Node {
 public:
  explicit Node() = default;
  explicit Node(NodeKind kind, Token* tok) : kind_(kind), tok_(tok) {}
  // whether the node is point.
  bool IsPointerNode();
  // inference the node type.
  void TypeInfer();
    // Report an error based on tok.
  void ErrorTok(const char* fmt, ...);

 public:
  // create const node.
  static Node* CreateConstNode(long val, Token* node_name);
  // create var node.
  static Node* CreateVarNode(Object* var, Token* node_name);
  // create identify node.
  static Node* CreateIdentNode(Token* node_name);
  // create call node
  static Node* CreateCallNode(Token* call_name, Node* args);
  // create unary expration node.
  static Node* CreateUnaryNode(NodeKind kind, Token* node_name, Node* op);
  // create binary expration node.
  static Node* CreateBinaryNode(NodeKind kind, Token* node_name, Node* op_left, Node* op_right);
  // create add expration(contain point arithmatic calculation) node.
  static Node* CreateAddNode(Token* node_name, Node* op_left, Node* op_right);
  // create subtract expration(contain point arithmatic calculation) node.
  static Node* CreateSubNode(Token* node_name, Node* op_left, Node* op_right);
  // create IF expration node.
  static Node* CreateIFNode(Token* node_name, Node* cond, Node* then, Node* els);
  // create for expration node.
  static Node* CreateForNode(Token* node_name, Node* init, Node* cond, Node* inc, Node* then);
  // create block expration node.
  static Node* CreateBlockNode(Token* node_name, Node* body);
  // free the node list.
  static void NodeListFree(Node* node);
  // parsing token list and generate AST.
  // program = stmt*
  static Node* Program(Token** rest, Token* tok);

 private:
  // ----------------Parse Function------------------
  // compound-stmt = (declaration | stmt)* "}"
  static Node* CompoundStmt(Token** rest, Token* tok);
  // declaration = declspec (
  //                 declarator ( "=" expr)?
  //                 ("," declarator ("=" expr)? ) * )? ";"
  static Node* Declaration(Token** rest, Token* tok);
  // declspec = "int"
  static Type* Declspec(Token** rest, Token* tok);
  // declarator = "*"* ident type-suffix
  static Type* Declarator(Token** rest, Token* tok, Type* ty);
  // type-suffix = "(" func-params | "[" num "]" | ɛ
  static Type* TypeSuffix(Token** rest, Token* tok, Type* ty);
  // func-param = param ("," param) *
  // param = declspec declarator
  static Type* FunctionParam(Token** rest, Token* tok, Type* ty);
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
  // postfix = primary ("[" Expr "]")*
  static Node* Postfix(Token** rest, Token* tok);
  // primary = "(" expr ")" | "sizeof" unary | ident | num
  static Node* Primary(Token** rest, Token* tok);
  // function = ident "(" (assign ("," assign)*)? ")"
  static Node* Call(Token** rest, Token* tok);
  // post-order for AST delete.
  static void NodeFree(Node* node);

  friend class CodeGenerator;
  friend class Object;

  // Node kind
  NodeKind kind_ = ND_END;
  // Representative node, node name
  Token* tok_ = nullptr;
  // node type
  Type* ty_ = nullptr;

  // for compound-stmt
  Node* next_ = nullptr;

  // for operation +-/*
  Node* lhs_ = nullptr;  // left-head side
  Node* rhs_ = nullptr;  // right-head side

  // for block
  Node* body_ = nullptr;

  // for "if" statement
  Node* cond_ = nullptr;
  Node* then_ = nullptr;
  Node* els_ = nullptr;

  // for "for" statement
  Node* init_ = nullptr;
  Node* inc_ = nullptr;
  // ------ function ------;
  char* call_ = nullptr;
  Node* args_ = nullptr;
  //  ------ for Var ------;
  Object* var_ = nullptr;
  //  ------ for const ------;
  long val_ = 0;
};

#endif  // !NODE_GRUAD
