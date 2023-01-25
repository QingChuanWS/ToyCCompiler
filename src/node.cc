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

extern ObjectPtr locals;

NodePtr Node::CreateConstNode(long val, TokenPtr tok) {
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
  op_left->TypeInfer();
  op_right->TypeInfer();
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
  op_left->TypeInfer();
  op_right->TypeInfer();
  // num - num
  if (!op_left->IsPointerNode() && !op_right->IsPointerNode()) {
    NodePtr res = CreateBinaryNode(ND_SUB, node_name, op_left, op_right);
    res->ty = ty_int;
    return res;
  }
  // ptr - ptr
  if (op_left->IsPointerNode() && op_right->IsPointerNode()) {
    // careful curisive call.
    NodePtr sub = CreateBinaryNode(ND_SUB, node_name, op_left, op_right);
    NodePtr factor = CreateConstNode(op_left->ty->GetBase()->Size(), node_name);
    NodePtr res = CreateBinaryNode(ND_DIV, node_name, sub, factor);
    res->ty = ty_int;
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
  parent->TypeInfer();
  if (!parent->ty->IsStruct() && !parent->ty->IsUnion()) {
    node_name->ErrorTok("not a struct.");
  }

  NodePtr res = CreateUnaryNode(ND_MUMBER, node_name, parent);
  res->mem = parent->ty->GetStructMember(node_name);
  return res;
}

void Node::TypeInfer() {
  if (this->ty != nullptr) {
    return;
  }

  if (lhs != nullptr) {
    lhs->TypeInfer();
  }
  if (rhs != nullptr) {
    rhs->TypeInfer();
  }
  if (cond != nullptr) {
    cond->TypeInfer();
  }
  if (then != nullptr) {
    then->TypeInfer();
  }
  if (els != nullptr) {
    els->TypeInfer();
  }
  if (init != nullptr) {
    init->TypeInfer();
  }
  if (inc != nullptr) {
    inc->TypeInfer();
  }

  for (NodePtr n = body; n != nullptr; n = n->next) {
    n->TypeInfer();
  }
  for (NodePtr n = args; n != nullptr; n = n->next) {
    n->TypeInfer();
  }

  switch (kind) {
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_NEG:
      ty = lhs->ty;
      return;
    case ND_ASSIGN:
      if (lhs->ty->IsArray()) {
        lhs->name->ErrorTok("not an lvalue");
      }
      ty = lhs->ty;
      return;
    case ND_EQ:
    case ND_NE:
    case ND_LE:
    case ND_LT:
    case ND_NUM:
    case ND_CALL:
      ty = ty_int;
      return;
    case ND_VAR:
      ty = var->GetType();
      return;
    case ND_COMMON:
      ty = rhs->ty;
      return;
    case ND_MUMBER:
      ty = mem->ty;
      return;
    case ND_ADDR:
      if (lhs->ty->IsArray()) {
        ty = Type::CreatePointerType(lhs->ty->GetBase());
      } else {
        ty = Type::CreatePointerType(lhs->ty);
      }
      return;
    case ND_DEREF:
      if (!lhs->ty->IsPointer()) {
        name->ErrorTok("invalid pointer reference!");
      }
      ty = lhs->ty->GetBase();
      return;
    case ND_STMT_EXPR:
      if (body != nullptr) {
        NodePtr stmt = body;
        while (stmt->next != nullptr) {
          stmt = stmt->next;
        }
        if (stmt->kind == ND_EXPR_STMT) {
          ty = stmt->lhs->ty;
          return;
        }
      }
      name->ErrorTok("statement expression return void is not supported.");
    default:
      return;
  }
}

bool Node::IsPointerNode() { return ty->IsPointer(); }

