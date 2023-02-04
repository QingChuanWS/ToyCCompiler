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

NodePtr Node::CreateVarNode(ObjectPtr var, TokenPtr tok) {
  NodePtr node = std::make_shared<Node>(ND_VAR, tok);
  node->var = var;
  return node;
}

NodePtr Node::CreateIdentNode(TokenPtr tok) {
  ObjectPtr var = tok->FindVar();
  if (var == nullptr) {
    tok->ErrorTok("undefined variable.");
  }
  return CreateVarNode(var, tok);
}

NodePtr Node::CreateCallNode(TokenPtr call_name, NodePtr args) {
  NodePtr call_node = std::make_shared<Node>(ND_CALL, call_name);
  call_node->call = call_name->GetIdent();
  call_node->args = args;
  return call_node;
}

NodePtr Node::CreateUnaryNode(NodeKind kind, TokenPtr node_name, NodePtr op) {
  NodePtr res = std::make_shared<Node>(kind, node_name);
  res->lhs = op;
  return res;
}

NodePtr Node::CreateAddNode(TokenPtr node_name, NodePtr op_left, NodePtr op_right) {
  TypeInfer(op_left);
  TypeInfer(op_right);
  if (!op_left->IsPointerNode() && !op_right->IsPointerNode()) {
    NodePtr res = CreateBinaryNode(ND_ADD, node_name, op_left, op_right);
    res->ty = ty_int;
    return res;
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
  NodePtr factor = CreateConstNode(op_left->ty->GetBase()->Size(), node_name);
  NodePtr real_num = CreateBinaryNode(ND_MUL, node_name, op_right, factor);
  NodePtr res = CreateBinaryNode(ND_ADD, node_name, op_left, real_num);
  return res;
}

NodePtr Node::CreateSubNode(TokenPtr node_name, NodePtr op_left, NodePtr op_right) {
  TypeInfer(op_left);
  TypeInfer(op_right);
  // num - num
  if (!op_left->IsPointerNode() && !op_right->IsPointerNode()) {
    return CreateBinaryNode(ND_SUB, node_name, op_left, op_right);
    ;
  }
  // ptr - ptr
  if (op_left->IsPointerNode() && op_right->IsPointerNode()) {
    // careful curisive call.
    NodePtr sub = CreateBinaryNode(ND_SUB, node_name, op_left, op_right);
    sub->ty = ty_long;
    NodePtr factor = CreateConstNode(op_left->ty->GetBase()->Size(), node_name);
    NodePtr res = CreateBinaryNode(ND_DIV, node_name, sub, factor);
    res->ty = ty_long;
    return res;
  }
  if (!op_left->IsPointerNode() && op_right->IsPointerNode()) {
    node_name->ErrorTok("Invalid Operands.");
  }
  // ptr - num
  NodePtr factor = CreateConstNode(op_left->ty->GetBase()->Size(), node_name);
  NodePtr real_num = CreateBinaryNode(ND_MUL, node_name, op_right, factor);
  NodePtr res = CreateBinaryNode(ND_SUB, node_name, op_left, real_num);
  return res;
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
  TypeInfer(parent);
  if (!parent->ty->IsStruct() && !parent->ty->IsUnion()) {
    node_name->ErrorTok("not a struct.");
  }

  NodePtr res = CreateUnaryNode(ND_MUMBER, node_name, parent);
  res->mem = parent->ty->GetStructMember(node_name);
  return res;
}

NodePtr Node::CreateCastNode(TokenPtr node_name, NodePtr expr, TypePtr ty) {
  TypeInfer(expr);

  NodePtr res = std::make_shared<Node>(ND_CAST, node_name);
  res->lhs = expr;
  res->ty = ty;
  return res;
}

void Node::TypeInfer(NodePtr node) {
  if (node == nullptr || node->ty != nullptr) {
    return;
  }

  TypeInfer(node->lhs);
  TypeInfer(node->rhs);
  TypeInfer(node->cond);
  TypeInfer(node->then);
  TypeInfer(node->els);
  TypeInfer(node->init);
  TypeInfer(node->inc);

  for (NodePtr n = node->body; n != nullptr; n = n->next) {
    TypeInfer(n);
  }
  for (NodePtr n = node->args; n != nullptr; n = n->next) {
    TypeInfer(n);
  }

  switch (node->kind) {
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_NEG:
      node->ty = node->lhs->ty;
      return;
    case ND_ASSIGN:
      if (node->lhs->IsArray()) {
        node->lhs->Error("not an lvalue");
      }
      node->ty = node->lhs->ty;
      return;
    case ND_EQ:
    case ND_NE:
    case ND_LE:
    case ND_LT:
    case ND_NUM:
    case ND_CALL:
      node->ty = ty_long;
      return;
    case ND_VAR:
      node->ty = node->var->GetType();
      return;
    case ND_COMMON:
      node->ty = node->rhs->ty;
      return;
    case ND_MUMBER:
      node->ty = node->mem->ty;
      return;
    case ND_ADDR:
      if (node->lhs->IsArray()) {
        node->ty = Type::CreatePointerType(node->lhs->ty->GetBase());
      } else {
        node->ty = Type::CreatePointerType(node->lhs->ty);
      }
      return;
    case ND_DEREF:
      if (!node->lhs->IsPointerNode()) {
        node->Error("invalid pointer reference!");
      }
      if (node->lhs->ty->GetBase()->IsVoid()) {
        node->Error("dereferencing a void pointer.");
      }
      node->ty = node->lhs->ty->GetBase();
      return;
    case ND_STMT_EXPR:
      if (node->body != nullptr) {
        NodePtr stmt = node->body;
        while (stmt->next != nullptr) {
          stmt = stmt->next;
        }
        if (stmt->kind == ND_EXPR_STMT) {
          node->ty = stmt->lhs->ty;
          return;
        }
      }
      node->Error("statement expression return void is not supported.");
    default:
      return;
  }
}
