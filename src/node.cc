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
  auto node = std::make_shared<Node>(NodeKind::ND_NUM, tok);
  node->val = val;
  return node;
}

NodePtr Node::CreateLongConstNode(int64_t val, TokenPtr node_name) {
  auto res = CreateConstNode(val, node_name);
  res->ty = ty_long;
  return res;
}

NodePtr Node::CreateVarNode(ObjectPtr var, TokenPtr tok) {
  auto node = std::make_shared<Node>(NodeKind::ND_VAR, tok);
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
  auto call_node = std::make_shared<Node>(NodeKind::ND_CALL, call_name);
  call_node->call = call_name->GetIdent();
  call_node->args = args;
  call_node->fun_ty = func_ty;
  call_node->ty = func_ty->return_ty;
  return call_node;
}

NodePtr Node::CreateUnaryNode(NodeKind kind, TokenPtr node_name, NodePtr op) {
  auto res = std::make_shared<Node>(kind, node_name);
  res->lhs = op;
  return res;
}

NodePtr Node::CreateAddNode(TokenPtr node_name, NodePtr op_left, NodePtr op_right) {
  Type::TypeInfer(op_left);
  Type::TypeInfer(op_right);
  if (!op_left->IsPointerNode() && !op_right->IsPointerNode()) {
    return CreateBinaryNode(NodeKind::ND_ADD, node_name, op_left, op_right);
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
  NodePtr real_num = CreateBinaryNode(NodeKind::ND_MUL, node_name, op_right, factor);
  NodePtr res = CreateBinaryNode(NodeKind::ND_ADD, node_name, op_left, real_num);
  return res;
}

NodePtr Node::CreateSubNode(TokenPtr node_name, NodePtr op_left, NodePtr op_right) {
  Type::TypeInfer(op_left);
  Type::TypeInfer(op_right);
  // num - num
  if (!op_left->IsPointerNode() && !op_right->IsPointerNode()) {
    return CreateBinaryNode(NodeKind::ND_SUB, node_name, op_left, op_right);
  }
  // ptr - ptr
  else if (op_left->IsPointerNode() && op_right->IsPointerNode()) {
    // careful curisive call.
    NodePtr sub = CreateBinaryNode(NodeKind::ND_SUB, node_name, op_left, op_right);
    sub->ty = ty_long;
    NodePtr factor = CreateLongConstNode(op_left->ty->GetBase()->Size(), node_name);
    return CreateBinaryNode(NodeKind::ND_DIV, node_name, sub, factor);
  } else if (!op_left->IsPointerNode() && op_right->IsPointerNode()) {
    node_name->ErrorTok("Invalid Operands.");
    return nullptr;
  } else {
    // ptr - num
    NodePtr factor = CreateLongConstNode(op_left->ty->GetBase()->Size(), node_name);
    NodePtr real_num = CreateBinaryNode(NodeKind::ND_MUL, node_name, op_right, factor);
    Type::TypeInfer(real_num);
    NodePtr res = CreateBinaryNode(NodeKind::ND_SUB, node_name, op_left, real_num);
    return res;
  }
}

NodePtr Node::CreateBinaryNode(NodeKind kind, TokenPtr node_name, NodePtr op_left,
                               NodePtr op_right) {
  auto res = std::make_shared<Node>(kind, node_name);
  res->lhs = op_left;
  res->rhs = op_right;
  return res;
}

NodePtr Node::CreateIfNode(TokenPtr node_name, NodePtr cond, NodePtr then, NodePtr els) {
  auto res = std::make_shared<Node>(NodeKind::ND_IF, node_name);
  res->cond = cond;
  res->then = then;
  res->els = els;
  return res;
}

NodePtr Node::CreateForNode(TokenPtr node_name, NodePtr init, NodePtr cond, NodePtr inc,
                            NodePtr then) {
  auto res = std::make_shared<Node>(NodeKind::ND_FOR, node_name);
  res->init = init;
  res->cond = cond;
  res->inc = inc;
  res->then = then;
  return res;
}

NodePtr Node::CreateBlockNode(NodeKind kind, TokenPtr node_name, NodePtr body) {
  auto res = std::make_shared<Node>(kind, node_name);
  res->body = body;
  return res;
}

// create struct member node.
NodePtr Node::CreateMemberNode(NodePtr parent, TokenPtr node_name) {
  Type::TypeInfer(parent);
  if (!parent->ty->Is<TY_STRUCT>() && !parent->ty->Is<TY_UNION>()) {
    node_name->ErrorTok("not a struct.");
  }

  NodePtr res = CreateUnaryNode(NodeKind::ND_MUMBER, node_name, parent);
  res->mem = parent->ty->GetStructMember(node_name);
  return res;
}

NodePtr Node::CreateCastNode(TokenPtr node_name, NodePtr expr, TypePtr ty) {
  Type::TypeInfer(expr);

  auto res = std::make_shared<Node>(NodeKind::ND_CAST, node_name);
  res->lhs = expr;
  res->ty = ty;
  return res;
}

// Convert `A op= B` to `tmp = &A, *tmp = *tmp op B`
// where tmp is a fresh pointer variable.
NodePtr Node::CreateCombinedNode(NodePtr binary) {
  Type::TypeInfer(binary->lhs);
  Type::TypeInfer(binary->rhs);

  TokenPtr root_name = binary->name;
  // generate fresh pointer variable.
  ObjectPtr var = Object::CreateLocalVar("", Type::CreatePointerType(binary->lhs->ty), &locals);
  // &A
  NodePtr lhs_addr = CreateUnaryNode(ND_ADDR, root_name, binary->lhs);
  // tmp = &A
  NodePtr expr1 = CreateBinaryNode(ND_ASSIGN, root_name, CreateVarNode(var, root_name), lhs_addr);

  // *tmp(as left val)
  NodePtr deref_lval = CreateUnaryNode(ND_DEREF, root_name, CreateVarNode(var, root_name));
  // *tmp(as right val)
  NodePtr deref_rval = CreateUnaryNode(ND_DEREF, root_name, CreateVarNode(var, root_name));
  // *tmp op rhs
  NodePtr compute = CreateBinaryNode(binary->kind, root_name, deref_rval, binary->rhs);
  // *tmp = *tmp op rhs.
  NodePtr expr2 = CreateBinaryNode(ND_ASSIGN, root_name, deref_lval, compute);
  return CreateBinaryNode(ND_COMMON, root_name, expr1, expr2);
}

// Convert A++ to `(typeof A)(A += 1) -1`
NodePtr Node::CreateIncdecNode(TokenPtr name, NodePtr prefix, int addend) {
  Type::TypeInfer(prefix);
  // A + 1
  NodePtr add = CreateAddNode(name, prefix, CreateConstNode(addend, name));
  // A += 1
  NodePtr add_assgin = CreateCombinedNode(add);
  // (A += 1) - 1
  NodePtr sub = CreateAddNode(name, add_assgin, CreateConstNode(-addend, name));
  // (typeof A)(A += 1) -1
  return CreateCastNode(name, sub, prefix->ty);
}
