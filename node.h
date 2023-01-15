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

#include <cstddef>
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
  static NodePtr CreateIfNode(TokenPtr node_name, NodePtr cond, NodePtr then, NodePtr els);
  // create for expration node.
  static NodePtr CreateForNode(TokenPtr node_name, NodePtr init, NodePtr cond, NodePtr inc,
                               NodePtr then);
  // create block expression node.
  static NodePtr CreateBlockNode(NodeKind kind, TokenPtr node_name, NodePtr body);

 private:
  friend class CodeGenerator;
  friend class Parser;

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
