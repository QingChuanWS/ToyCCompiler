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
#include <cstdint>
#include <cstring>
#include <memory>

#include "object.h"
#include "scope.h"
#include "token.h"
#include "tools.h"
#include "type.h"
#include "utils.h"

NodePtrVec goto_list{};
NodePtrVec label_list{};

NodePtr cur_swt = nullptr;

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

NodePtr Node::CreateIfNode(NodeKind kind, TokenPtr node_name, NodePtr cond, NodePtr then,
                           NodePtr els) {
  auto res = std::make_shared<Node>(kind, node_name);
  res->cond = cond;
  res->then = then;
  res->els = els;
  return res;
}

NodePtr Node::CreateForNode(TokenPtr node_name, NodePtr init, NodePtr cond, NodePtr inc,
                            NodePtr then, String brk_label, String cnt_label) {
  auto res = std::make_shared<Node>(NodeKind::ND_FOR, node_name);
  res->init = init;
  res->cond = cond;
  res->inc = inc;
  res->then = then;
  res->break_label = brk_label;
  res->continue_label = cnt_label;
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
NodePtr Node::CreateCombinedNode(NodePtr binary, ObjectList& locals) {
  Type::TypeInfer(binary->lhs);
  Type::TypeInfer(binary->rhs);

  TokenPtr root_name = binary->name;
  // generate fresh pointer variable.
  ObjectPtr var = Object::CreateLocalVar("", Type::CreatePointerType(binary->lhs->ty), locals);
  // &A
  NodePtr lhs_addr = CreateUnaryNode(ND_ADDR, root_name, binary->lhs);
  // tmp = &A
  NodePtr expr1 = CreateBinaryNode(ND_ASSIGN, root_name, CreateVarNode(var, root_name), lhs_addr);

  // *tmp(as left val)
  // the fresh point should be indenpent by each other.
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
NodePtr Node::CreateIncdecNode(TokenPtr name, NodePtr prefix, int addend, ObjectList& locals) {
  Type::TypeInfer(prefix);
  // A + 1
  NodePtr add = CreateAddNode(name, prefix, CreateConstNode(addend, name));
  // A += 1
  NodePtr add_assgin = CreateCombinedNode(add, locals);
  // (A += 1) - 1
  NodePtr sub = CreateAddNode(name, add_assgin, CreateConstNode(-addend, name));
  // (typeof A)(A += 1) -1
  return CreateCastNode(name, sub, prefix->ty);
}

NodePtr Node::CreateGotoNode(TokenPtr label, String label_name, bool need_update) {
  auto res = std::make_shared<Node>(ND_GOTO, label);
  if (need_update) {
    res->label = label_name;
    goto_list.push_back(res);
  } else {
    res->unique_label = label_name;
  }
  return res;
}

NodePtr Node::CreateGotoLableNode(TokenPtr label_name, NodePtr body) {
  auto res = std::make_shared<Node>(ND_LABEL, label_name);
  res->label = label_name->GetIdent();
  res->unique_label = CreateUniqueName();
  res->body = body;
  label_list.push_back(res);
  return res;
}

// This function matches gotos with labels.
//
// We cannot resolve gotos as we parse a function because gotos
// can refer a label that appears later in the function.
// So, we need to do this after we parse the entire function.
void Node::UpdateGotoLabel() {
  for (auto g : goto_list) {
    for (auto l : label_list) {
      if (g->label == l->label) {
        g->unique_label = l->unique_label;
        break;
      }
    }
    if (g->unique_label.empty()) {
      Token::GetNext<1>(g->name)->ErrorTok("use of undeclared label.");
    }
  }
  goto_list.clear();
  label_list.clear();
}

NodePtr Node::CreateSwitchNode(TokenPtr node_name, NodePtr cond) {
  NodePtr res = std::make_shared<Node>(ND_SWITCH, node_name);
  res->cond = cond;
  return res;
}

NodePtr Node::CreateCaseNode(TokenPtr node_name, int64_t val, NodePtr body) {
  NodePtr res = std::make_shared<Node>(ND_CASE, node_name);
  res->label = CreateUniqueName();
  res->val = val;
  res->body = body;
  return res;
}

NodePtr Node::CreateDefaultNode(TokenPtr node_name, NodePtr body) {
  NodePtr res = std::make_shared<Node>(ND_CASE, node_name);
  res->label = CreateUniqueName();
  res->body = body;
  return res;
}

int64_t Node::Eval(NodePtr node) {
  Type::TypeInfer(node);

  switch (node->kind) {
    case ND_ADD:
      return Eval(node->lhs) + Eval(node->rhs);
    case ND_SUB:
      return Eval(node->lhs) - Eval(node->rhs);
    case ND_MUL:
      return Eval(node->lhs) * Eval(node->rhs);
    case ND_DIV:
      return Eval(node->lhs) / Eval(node->rhs);
    case ND_NEG:
      return -Eval(node->lhs);
    case ND_MOD:
      return Eval(node->lhs) % Eval(node->rhs);
    case ND_BITAND:
      return Eval(node->lhs) & Eval(node->rhs);
    case ND_BITOR:
      return Eval(node->lhs) | Eval(node->rhs);
    case ND_BITXOR:
      return Eval(node->lhs) ^ Eval(node->rhs);
    case ND_SHL:
      return Eval(node->lhs) << Eval(node->rhs);
    case ND_SHR:
      return Eval(node->lhs) >> Eval(node->rhs);
    case ND_EQ:
      return Eval(node->lhs) == Eval(node->rhs);
    case ND_NE:
      return Eval(node->lhs) != Eval(node->rhs);
    case ND_LT:
      return Eval(node->lhs) < Eval(node->rhs);
    case ND_LE:
      return Eval(node->lhs) <= Eval(node->rhs);
    case ND_COND:
      return Eval(node->cond) ? Eval(node->then) : Eval(node->els);
    case ND_COMMON:
      return Eval(node->rhs);
    case ND_NOT:
      return !Eval(node->lhs);
    case ND_BITNOT:
      return ~Eval(node->lhs);
    case ND_LOGAND:
      return Eval(node->lhs) && Eval(node->rhs);
    case ND_LOGOR:
      return Eval(node->lhs) || Eval(node->rhs);
    case ND_CAST:
      if (node->ty->IsInteger()) {
        switch (node->ty->Size()) {
          case 1:
            return (uint8_t)Eval(node->lhs);
          case 2:
            return (uint16_t)Eval(node->lhs);
          case 4:
            return (uint64_t)Eval(node->lhs);
        }
      }
      return Eval(node->lhs);
    case ND_NUM:
      return node->val;
    default:
      node->name->ErrorTok("not a complier-time contant");
      return -1;
  }
}