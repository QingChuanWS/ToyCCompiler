/*
 * This project is exclusively owned by QingChuanWS and shall not be used for
 * commercial and profitting purpose without QingChuanWS's permission.
 *
 * @ Author: bingshan45@163.com
 * @ Github: https://github.com/QingChuanWS
 * @ Description:
 *
 * Copyright (c) 2023 by QingChuanWS, All Rights Reserved.
 */

#ifndef NODE_GRUAD
#define NODE_GRUAD

#include <cstddef>
#include <memory>
#include <string>

#include "object.h"
#include "struct.h"
#include "token.h"
#include "tools.h"
#include "type.h"
#include "utils.h"

enum NodeKind {
  ND_ADD,        // +
  ND_SUB,        // -
  ND_MUL,        // *
  ND_DIV,        // /
  ND_MOD,        // %
  ND_NEG,        // unary -
  ND_EQ,         // ==
  ND_NE,         // !=
  ND_LT,         // <
  ND_LE,         // <=
  ND_NUM,        // number
  ND_ASSIGN,     // =
  ND_COMMON,     // ,
  ND_MUMBER,     // .(struct member asscess)
  ND_ADDR,       // unary &
  ND_DEREF,      // *
  ND_NOT,        // !
  ND_BITNOT,     // ~
  ND_BITAND,     // &
  ND_BITOR,      // |
  ND_BITXOR,     // ^
  ND_LOGAND,     // &&
  ND_LOGOR,      // ||
  ND_EXPR_STMT,  // expression statement
  ND_STMT_EXPR,  // statement expression
  ND_RETURN,     // return
  ND_BLOCK,      // { ... }
  ND_GOTO,       // goto
  ND_LABEL,      // labeled statement
  ND_CALL,       // Function call
  ND_IF,         // if
  ND_FOR,        // for and while
  ND_VAR,        // variable
  ND_CAST,       // type cast
  ND_END,
};

class Node {
 public:
  Node(NodeKind kind, TokenPtr tok) : kind(kind), name(tok) {}
  // whether the node is point.
  inline bool IsPointerNode() const { return ty->Is<TY_PRT>(); }
  // whether the node is array node.
  inline bool IsArrayNode() const { return ty->Is<TY_ARRAY>(); }
  // error print
  void Error(const char* fmt, ...) const;

  // create const node with type == ty_long.
  static NodePtr CreateLongConstNode(int64_t val, TokenPtr node_name);
  // create const node.
  static NodePtr CreateConstNode(int64_t val, TokenPtr node_name);
  // create var node.
  static NodePtr CreateVarNode(ObjectPtr var, TokenPtr node_name);
  // create identify node.
  static NodePtr CreateIdentNode(TokenPtr node_name);
  // create call node
  static NodePtr CreateCallNode(TokenPtr call_name, NodePtr args, TypePtr ret_ty);
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
                               NodePtr then, String brk_label);
  // create block expression node.
  static NodePtr CreateBlockNode(NodeKind kind, TokenPtr node_name, NodePtr body);
  // create struct member node.
  static NodePtr CreateMemberNode(NodePtr parent, TokenPtr node_name);
  // create cast node.
  static NodePtr CreateCastNode(TokenPtr node_name, NodePtr expr, TypePtr ty);
  // create a combined arithmatic node, such as "+=", "-="...
  static NodePtr CreateCombinedNode(NodePtr binary);
  // create a post inc and dec node.
  static NodePtr CreateIncdecNode(TokenPtr node_name, NodePtr prefix, int addend);
  // create a goto node.
  static NodePtr CreateGotoNode(TokenPtr label, String label_name, bool need_match = true);
  // create a goto label node.
  static NodePtr CreateGotoLableNode(TokenPtr label, NodePtr body);
  // update goto label
  static void UpdateGotoLabel();

 private:
  friend class CodeGenerator;
  friend class Parser;
  friend class Type;

  // Node kind
  NodeKind kind = NodeKind::ND_END;
  // Representative node, node name
  TokenPtr name = nullptr;
  // node type
  TypePtr ty = nullptr;

  // compound-stmt
  NodePtr next = nullptr;

  //  operation +-/*
  NodePtr lhs = nullptr;  // left-head side
  NodePtr rhs = nullptr;  // right-head side

  // block or statment expression.
  NodePtr body = nullptr;

  // "if" statement
  NodePtr cond = nullptr;
  NodePtr then = nullptr;
  NodePtr els = nullptr;

  // "for" statement
  NodePtr init = nullptr;
  NodePtr inc = nullptr;
  String break_label = "";  // break;

  // struct member access.
  MemberPtr mem = nullptr;

  // ------ function ------;
  String call = String();
  TypePtr fun_ty = nullptr;
  NodePtr args = nullptr;

  // ----- goto ------;
  String label = "";
  String unique_label = "";

  //  ------  Var ------;
  ObjectPtr var = nullptr;

  //  ------  const ------;
  int64_t val = 0;
};

#endif  // !NODE_GRUAD
