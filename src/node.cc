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

#include "node.h"

#include <cstdarg>
#include <cstddef>
#include <cstring>
#include <memory>

#include "object.h"
#include "scope.h"
#include "token.h"
#include "tools.h"
#include "type.h"
#include "utils.h"

void Node::Error(const char* fmt, ...) const {
  va_list ap;
  va_start(ap, fmt);
  name->ErrorTok(fmt, ap);
}

NodePtr Node::CreateConstNode(int64_t val, TokenPtr tok) {
  NodePtr node = std::make_shared<Node>(ND_NUM, tok);
  node->val = val;
  return node;
}

NodePtr Node::CreateLongConstNode(int64_t val, TokenPtr node_name) {
  NodePtr res = CreateConstNode(val, node_name);
  res->ty = ty_long;
  return res;
}

NodePtr Node::CreateVarNode(ObjectPtr var, TokenPtr tok) {
  NodePtr node = std::make_shared<Node>(ND_VAR, tok);
  node->var = var;
  return node;
}

NodePtr Node::CreateIdentNode(TokenPtr tok) {
  VarScopePtr sc = Scope::FindVarScope(tok->GetIdent());
  if (!sc || (!sc->var && !sc->IsEnum())) {
    tok->ErrorTok("undefined variable.");
  }
  NodePtr res = nullptr;
  if (sc->var) {
    res = CreateVarNode(sc->var, tok);
  } else {
    res = CreateConstNode(sc->GetEnumList(), tok);
  }
  return res;
}

NodePtr Node::CreateCallNode(TokenPtr call_name, NodePtr args, TypePtr func_ty) {
  NodePtr call_node = std::make_shared<Node>(ND_CALL, call_name);
  call_node->call = call_name->GetIdent();
  call_node->args = args;
  call_node->fun_ty = func_ty;
  call_node->ty = func_ty->return_ty;
  return call_node;
}

NodePtr Node::CreateUnaryNode(NodeKind kind, TokenPtr node_name, NodePtr op) {
  NodePtr res = std::make_shared<Node>(kind, node_name);
  res->lhs = op;
  return res;
}

NodePtr Node::CreateAddNode(TokenPtr node_name, NodePtr op_left, NodePtr op_right) {
  Type::TypeInfer(op_left);
  Type::TypeInfer(op_right);
  if (!op_left->IsPointerNode() && !op_right->IsPointerNode()) {
    return CreateBinaryNode(ND_ADD, node_name, op_left, op_right);
  }
  // ptr + ptr
  if (op_left->IsPointerNode() && op_right->IsPointerNode()) {
    node_name->ErrorTok("Invalid Operands.");
  }
  // cononical 'num + ptr' to 'ptr + num'.
  if (!op_left->IsPointerNode() && op_right->IsPointerNode()) {
    NodePtr tmp = op_left;
    op_left = op_right;
    op_right = tmp;
  }
  // ptr + num
  NodePtr factor = CreateLongConstNode(op_left->ty->GetBase()->Size(), node_name);
  NodePtr real_num = CreateBinaryNode(ND_MUL, node_name, op_right, factor);
  NodePtr res = CreateBinaryNode(ND_ADD, node_name, op_left, real_num);
  return res;
}

NodePtr Node::CreateSubNode(TokenPtr node_name, NodePtr op_left, NodePtr op_right) {
  Type::TypeInfer(op_left);
  Type::TypeInfer(op_right);
  // num - num
  if (!op_left->IsPointerNode() && !op_right->IsPointerNode()) {
    return CreateBinaryNode(ND_SUB, node_name, op_left, op_right);
  }
  // ptr - ptr
  else if (op_left->IsPointerNode() && op_right->IsPointerNode()) {
    // careful curisive call.
    NodePtr sub = CreateBinaryNode(ND_SUB, node_name, op_left, op_right);
    sub->ty = ty_long;
    NodePtr factor = CreateLongConstNode(op_left->ty->GetBase()->Size(), node_name);
    return CreateBinaryNode(ND_DIV, node_name, sub, factor);
  } else if (!op_left->IsPointerNode() && op_right->IsPointerNode()) {
    node_name->ErrorTok("Invalid Operands.");
    return nullptr;
  } else {
    // ptr - num
    NodePtr factor = CreateLongConstNode(op_left->ty->GetBase()->Size(), node_name);
    NodePtr real_num = CreateBinaryNode(ND_MUL, node_name, op_right, factor);
    Type::TypeInfer(real_num);
    NodePtr res = CreateBinaryNode(ND_SUB, node_name, op_left, real_num);
    return res;
  }
}

NodePtr Node::CreateBinaryNode(NodeKind kind, TokenPtr node_name, NodePtr op_left,
                               NodePtr op_right) {
  NodePtr res = std::make_shared<Node>(kind, node_name);
  res->lhs = op_left;
  res->rhs = op_right;
  return res;
}

NodePtr Node::CreateIfNode(TokenPtr node_name, NodePtr cond, NodePtr then, NodePtr els) {
  NodePtr res = std::make_shared<Node>(ND_IF, node_name);
  res->cond = cond;
  res->then = then;
  res->els = els;
  return res;
}

NodePtr Node::CreateForNode(TokenPtr node_name, NodePtr init, NodePtr cond, NodePtr inc,
                            NodePtr then) {
  NodePtr res = std::make_shared<Node>(ND_FOR, node_name);
  res->init = init;
  res->cond = cond;
  res->inc = inc;
  res->then = then;
  return res;
}

NodePtr Node::CreateBlockNode(NodeKind kind, TokenPtr node_name, NodePtr body) {
  NodePtr res = std::make_shared<Node>(kind, node_name);
  res->body = body;
  return res;
}

// create struct member node.
NodePtr Node::CreateMemberNode(NodePtr parent, TokenPtr node_name) {
  Type::TypeInfer(parent);
  if (!parent->ty->Is<TY_STRUCT>() && !parent->ty->Is<TY_UNION>()) {
    node_name->ErrorTok("not a struct.");
  }

  NodePtr res = CreateUnaryNode(ND_MUMBER, node_name, parent);
  res->mem = parent->ty->GetStructMember(node_name);
  return res;
}

NodePtr Node::CreateCastNode(TokenPtr node_name, NodePtr expr, TypePtr ty) {
  Type::TypeInfer(expr);

  NodePtr res = std::make_shared<Node>(ND_CAST, node_name);
  res->lhs = expr;
  res->ty = ty;
  return res;
}
