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

#include <memory>
#include <string>

#include "object.h"
#include "token.h"
#include "tools.h"
#include "type.h"
#include "utils.h"

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
  ND_STMT_EXPR,  // statement expression
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
  Node(NodeKind kind, TokenPtr tok) : kind(kind), name(tok) {}
  // whether the node is point.
  bool IsPointerNode();
  // inference the node type.
  void TypeInfer();
  // Report an error based on tok.
  void ErrorTok(const char* fmt, ...);

 public:
  // create const node.
  static NodePtr CreateConstNode(long val, TokenPtr node_name);
  // create var node.
  static NodePtr CreateVarNode(ObjectPtr var, TokenPtr node_name);
  // create identify node.
  static NodePtr CreateIdentNode(TokenPtr node_name);
  // create call node
  static NodePtr CreateCallNode(TokenPtr call_name, NodePtr args);
  // create unary expration node.
  static NodePtr CreateUnaryNode(NodeKind kind, TokenPtr node_name, NodePtr op);
  // create binary expration node.
  static NodePtr CreateBinaryNode(NodeKind kind, TokenPtr node_name, NodePtr op_left,
                                  NodePtr op_right);
  // create add expration(contain point arithmatic calculation) node.
  static NodePtr CreateAddNode(TokenPtr node_name, NodePtr op_left, NodePtr op_right);
  // create subtract expration(contain point arithmatic calculation) node.
  static NodePtr CreateSubNode(TokenPtr node_name, NodePtr op_left, NodePtr op_right);
  // create IF expration node.
  static NodePtr CreateIFNode(TokenPtr node_name, NodePtr cond, NodePtr then, NodePtr els);
  // create for expration node.
  static NodePtr CreateForNode(TokenPtr node_name, NodePtr init, NodePtr cond, NodePtr inc,
                               NodePtr then);
  // create block expression node.
  static NodePtr CreateBlockNode(NodeKind kind, TokenPtr node_name, NodePtr body);
  // parsing token list and generate AST.
  // program = stmt*
  static NodePtr Program(TokenPtr* rest, TokenPtr tok);

 private:
  // ----------------Parse Function------------------
  // compound-stmt = (declaration | stmt)* "}"
  static NodePtr CompoundStmt(TokenPtr* rest, TokenPtr tok);
  // declaration = declspec (
  //                 declarator ( "=" expr)?
  //                 ("," declarator ("=" expr)? ) * )? ";"
  static NodePtr Declaration(TokenPtr* rest, TokenPtr tok);
  // declspec = "int"
  static TypePtr Declspec(TokenPtr* rest, TokenPtr tok);
  // declarator = "*"* ident type-suffix
  static TypePtr Declarator(TokenPtr* rest, TokenPtr tok, TypePtr ty);
  // type-suffix = "(" func-params | "[" num "]" | ɛ
  static TypePtr TypeSuffix(TokenPtr* rest, TokenPtr tok, TypePtr ty);
  // func-param = param ("," param) *
  // param = declspec declarator
  static TypePtr FunctionParam(TokenPtr* rest, TokenPtr tok, TypePtr ty);
  // stmt = "return" expr ";" |
  // "if" "(" expr ")" stmt ("else" stmt)? |
  // "for" "(" expr-stmt expr? ";" expr? ")" stmt |
  // "while" "(" expr ")" stmt |
  // "{" compuound-stmt |
  // expr-stmt
  static NodePtr Stmt(TokenPtr* rest, TokenPtr tok);
  // expr-stmt = expr ";"
  static NodePtr ExprStmt(TokenPtr* rest, TokenPtr tok);
  // expr = assign
  static NodePtr Expr(TokenPtr* rest, TokenPtr tok);
  // assign = equality ("=" assign)?
  static NodePtr Assign(TokenPtr* rest, TokenPtr tok);
  // equality = relational ("==" relational | "!=" relational)
  static NodePtr Equality(TokenPtr* rest, TokenPtr tok);
  // relational = add ("<" add | "<=" add | ">" add | ">=" add)
  static NodePtr Relational(TokenPtr* rest, TokenPtr tok);
  // add = mul ("+"mul | "-" mul)
  static NodePtr Add(TokenPtr* rest, TokenPtr tok);
  // mul = unary ("*" unary | "/" unary)
  static NodePtr Mul(TokenPtr* rest, TokenPtr tok);
  // unary = ("+" | "-" | "*" | "&") ? unary | primary
  static NodePtr Unary(TokenPtr* rest, TokenPtr tok);
  // postfix = primary ("[" Expr "]")*
  static NodePtr Postfix(TokenPtr* rest, TokenPtr tok);
  // primary = "(" "{" stmt+ "}" ")"
  //          |"(" expr ")" | "sizeof" unary | ident func-args? | str | num
  static NodePtr Primary(TokenPtr* rest, TokenPtr tok);
  // function = ident "(" (assign ("," assign)*)? ")"
  static NodePtr Call(TokenPtr* rest, TokenPtr tok);

  friend class CodeGenerator;
  friend class Object;

  // Node kind
  NodeKind kind = ND_END;
  // Representative node, node name
  TokenPtr name = nullptr;
  // node type
  TypePtr ty = nullptr;

  // for compound-stmt
  NodePtr next = nullptr;

  // for operation +-/*
  NodePtr lhs = nullptr;  // left-head side
  NodePtr rhs = nullptr;  // right-head side

  // for block or statment expression.
  NodePtr body = nullptr;

  // for "if" statement
  NodePtr cond = nullptr;
  NodePtr then = nullptr;
  NodePtr els = nullptr;

  // for "for" statement
  NodePtr init = nullptr;
  NodePtr inc = nullptr;
  
  // ------ function ------;
  String call = String();
  NodePtr args = nullptr;

  //  ------ for Var ------;
  ObjectPtr var = nullptr;
  
  //  ------ for const ------;
  long val = 0;
};

#endif  // !NODE_GRUAD
