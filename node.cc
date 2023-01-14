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
  ObjectPtr local_var = tok->FindLocalVar();
  ObjectPtr global_var = tok->FindGlobalVar();
  if (global_var == nullptr && local_var == nullptr) {
    tok->ErrorTok("undefined variable.");
  }
  if (local_var) {
    return CreateVarNode(local_var, tok);
  }
  return CreateVarNode(global_var, tok);
}

NodePtr Node::CreateCallNode(TokenPtr call_name, NodePtr args) {
  NodePtr call_node = std::make_shared<Node>(ND_CALL, call_name);
  call_node->call = String(call_name->loc, call_name->len);
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
  NodePtr factor = CreateConstNode(op_left->ty->base->size, node_name);
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
    NodePtr factor = CreateConstNode(op_left->ty->base->size, node_name);
    NodePtr res = CreateBinaryNode(ND_DIV, node_name, sub, factor);
    res->ty = ty_int;
    return res;
  }
  if (!op_left->IsPointerNode() && op_right->IsPointerNode()) {
    node_name->ErrorTok("Invalid Operands.");
  }
  // ptr - num
  NodePtr factor = CreateConstNode(op_left->ty->base->size, node_name);
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

NodePtr Node::CreateIFNode(TokenPtr node_name, NodePtr cond, NodePtr then, NodePtr els) {
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

NodePtr Node::Program(TokenPtr* rest, TokenPtr tok) {
  tok = tok->SkipToken("{");
  return CompoundStmt(rest, tok);
}

// compound-stmt  = (declaration | stmt)* "}"
NodePtr Node::CompoundStmt(TokenPtr* rest, TokenPtr tok) {
  NodePtr sub_expr = std::make_shared<Node>(ND_END, tok);
  NodePtr cur = sub_expr;

  while (!tok->Equal("}")) {
    if (tok->IsTypename()) {
      cur->next = Declaration(&tok, tok);
    } else {
      cur->next = Node::Stmt(&tok, tok);
    }
    cur = cur->next;
    cur->TypeInfer();
  }

  *rest = tok->next;
  return CreateBlockNode(ND_BLOCK, tok, sub_expr->next);
  ;
}

// declaration = declspec (
//                 declarator ( "=" expr)?
//                 ("," declarator ("=" expr)? ) * )? ";"
NodePtr Node::Declaration(TokenPtr* rest, TokenPtr tok) {
  TypePtr ty_base = Declspec(&tok, tok);

  NodePtr decl_expr = std::make_shared<Node>(ND_END, tok);
  NodePtr cur = decl_expr;
  int i = 0;

  while (!tok->Equal(";")) {
    if (i++ > 0) {
      tok = tok->SkipToken(",");
    }
    TypePtr ty = Declarator(&tok, tok, ty_base);
    ObjectPtr var = Object::CreateLocalVar(ty->name->GetIdent(), ty, &locals);
    if (!tok->Equal("=")) {
      continue;
    }
    NodePtr lhs = CreateVarNode(var, tok);
    NodePtr rhs = Assign(&tok, tok->next);
    NodePtr assgin_node = CreateBinaryNode(ND_ASSIGN, tok, lhs, rhs);
    cur = cur->next = CreateUnaryNode(ND_EXPR_STMT, tok, assgin_node);
  }

  *rest = tok->next;
  return CreateBlockNode(ND_BLOCK, tok, decl_expr->next);
  ;
}

// declspec = "char" | "int"
TypePtr Node::Declspec(TokenPtr* rest, TokenPtr tok) {
  if (tok->Equal("char")) {
    *rest = tok->SkipToken("char");
    return ty_char;
  }

  *rest = tok->SkipToken("int");
  return ty_int;
}

// declarator = "*"* ident type-suffix
TypePtr Node::Declarator(TokenPtr* rest, TokenPtr tok, TypePtr ty) {
  while (tok->Equal("*")) {
    ty = Type::CreatePointerType(ty);
    tok = tok->next;
  }
  *rest = tok;

  if (tok->kind != TK_IDENT) {
    tok->ErrorTok("expected a variable name.");
  }
  ty = TypeSuffix(rest, tok->next, ty);
  ty->name = tok;
  return ty;
}

// type-suffix = "(" func-params ")" | "[" num "]" type-suffix | ɛ
TypePtr Node::TypeSuffix(TokenPtr* rest, TokenPtr tok, TypePtr ty) {
  if (tok->Equal("(")) {
    return FunctionParam(rest, tok->next, ty);
  }
  if (tok->Equal("[")) {
    int len = tok->next->GetNumber();
    tok = tok->next->next->SkipToken("]");
    ty = TypeSuffix(rest, tok, ty);
    return Type::CreateArrayType(ty, len);
  }

  *rest = tok;
  return ty;
}

// func-param = param ("," param) *
// param = declspec declarator
TypePtr Node::FunctionParam(TokenPtr* rest, TokenPtr tok, TypePtr ty) {
  TypePtr params = std::make_shared<Type>(TY_END, 0);
  TypePtr cur = params;

  while (!tok->Equal(")")) {
    if (cur != params) {
      tok = tok->SkipToken(",");
    }
    TypePtr base_ty = Declspec(&tok, tok);
    TypePtr var_type = Declarator(&tok, tok, base_ty);
    cur->next = std::make_shared<Type>(*var_type);
    cur = cur->next;
  }
  ty = Type::CreateFunctionType(ty, params->next);

  *rest = tok->next;
  return ty;
}

// stmt = "return" expr ";" |
// "if" "(" expr ")" stmt ("else" stmt)? |
// "for" "(" expr-stmt expr? ";" expr? ")" stmt |
// "{" compuound-stmt |
// expr-stmt
NodePtr Node::Stmt(TokenPtr* rest, TokenPtr tok) {
  if (tok->Equal("return")) {
    NodePtr node = CreateUnaryNode(ND_RETURN, tok, Expr(&tok, tok->next));
    *rest = tok->SkipToken(";");
    return node;
  }

  if (tok->Equal("if")) {
    TokenPtr node_name = tok;
    tok = tok->next;
    NodePtr cond = Expr(&tok, tok->SkipToken("("));
    NodePtr then = Stmt(&tok, tok->SkipToken(")"));
    NodePtr els = nullptr;
    if (tok->Equal("else")) {
      els = Stmt(&tok, tok->next);
    }
    *rest = tok;
    return CreateIFNode(node_name, cond, then, els);
  }

  if (tok->Equal("for")) {
    TokenPtr node_name = tok;
    tok = tok->next->SkipToken("(");

    NodePtr init = ExprStmt(&tok, tok);
    NodePtr cond = nullptr;
    NodePtr inc = nullptr;

    if (!tok->Equal(";")) {
      cond = Expr(&tok, tok);
    }
    tok = tok->SkipToken(";");
    if (!tok->Equal(")")) {
      inc = Expr(&tok, tok);
    }
    tok = tok->SkipToken(")");
    return CreateForNode(node_name, init, cond, inc, Stmt(rest, tok));
  }

  if (tok->Equal("while")) {
    TokenPtr node_name = tok;
    tok = tok->next->SkipToken("(");

    NodePtr cond = Expr(&tok, tok);
    tok = tok->SkipToken(")");
    NodePtr then = Stmt(rest, tok);
    return CreateForNode(node_name, nullptr, cond, nullptr, then);
  }

  if (tok->Equal("{")) {
    return CompoundStmt(rest, tok->next);
  }

  return ExprStmt(rest, tok);
}

// expr-stmt = expr ";"
NodePtr Node::ExprStmt(TokenPtr* rest, TokenPtr tok) {
  if (tok->Equal(";")) {
    *rest = tok->next;
    return CreateBlockNode(ND_BLOCK, tok, nullptr);
  }
  NodePtr node = CreateUnaryNode(ND_EXPR_STMT, tok, Expr(&tok, tok));
  *rest = tok->SkipToken(";");
  return node;
}

// expr = assign
NodePtr Node::Expr(TokenPtr* rest, TokenPtr tok) { return Assign(rest, tok); }

// assign = equality ("=" assign)?
NodePtr Node::Assign(TokenPtr* rest, TokenPtr tok) {
  NodePtr node = Equality(&tok, tok);
  if (tok->Equal("=")) {
    return CreateBinaryNode(ND_ASSIGN, tok, node, Assign(rest, tok->next));
  }
  *rest = tok;
  return node;
}

// equality = relational ("==" relational | "!=" relational)
NodePtr Node::Equality(TokenPtr* rest, TokenPtr tok) {
  NodePtr node = Relational(&tok, tok);

  for (;;) {
    TokenPtr node_name = tok;
    if (tok->Equal("==")) {
      node = CreateBinaryNode(ND_EQ, node_name, node, Relational(&tok, tok->next));
      continue;
    }
    if (tok->Equal("!=")) {
      node = CreateBinaryNode(ND_NE, node_name, node, Relational(&tok, tok->next));
      continue;
    }

    *rest = tok;
    return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)
NodePtr Node::Relational(TokenPtr* rest, TokenPtr tok) {
  NodePtr node = Add(&tok, tok);

  for (;;) {
    TokenPtr node_name = tok;
    if (tok->Equal("<")) {
      node = CreateBinaryNode(ND_LT, node_name, node, Add(&tok, tok->next));
      continue;
    }
    if (tok->Equal("<=")) {
      node = CreateBinaryNode(ND_LE, node_name, node, Add(&tok, tok->next));
      continue;
    }
    if (tok->Equal(">")) {
      node = CreateBinaryNode(ND_LT, node_name, Add(&tok, tok->next), node);
      continue;
    }
    if (tok->Equal(">=")) {
      node = CreateBinaryNode(ND_LE, node_name, Add(&tok, tok->next), node);
      continue;
    }
    *rest = tok;
    return node;
  }
}

// add = mul ("+"mul | "-" mul)
NodePtr Node::Add(TokenPtr* rest, TokenPtr tok) {
  NodePtr node = Mul(&tok, tok);

  for (;;) {
    TokenPtr node_name = tok;
    if (tok->Equal("+")) {
      node = CreateAddNode(node_name, node, Mul(&tok, tok->next));
      continue;
    }
    if (tok->Equal("-")) {
      node = CreateSubNode(node_name, node, Mul(&tok, tok->next));
      continue;
    }
    *rest = tok;
    return node;
  }
}

NodePtr Node::Mul(TokenPtr* rest, TokenPtr tok) {
  NodePtr node = Unary(&tok, tok);

  for (;;) {
    TokenPtr node_name = tok;
    if (tok->Equal("*")) {
      node = CreateBinaryNode(ND_MUL, node_name, node, Primary(&tok, tok->next));
      continue;
    }
    if (tok->Equal("/")) {
      node = CreateBinaryNode(ND_DIV, node_name, node, Primary(&tok, tok->next));
      continue;
    }
    *rest = tok;
    return node;
  }
}

// unary = ("+" | "-" | "*" | "&") ? unary | primary
NodePtr Node::Unary(TokenPtr* rest, TokenPtr tok) {
  if (tok->Equal("+")) {
    return Unary(rest, tok->next);
  }
  if (tok->Equal("-")) {
    return CreateUnaryNode(ND_NEG, tok, Unary(rest, tok->next));
  }
  if (tok->Equal("&")) {
    return CreateUnaryNode(ND_ADDR, tok, Unary(rest, tok->next));
  }
  if (tok->Equal("*")) {
    return CreateUnaryNode(ND_DEREF, tok, Unary(rest, tok->next));
  }
  return Postfix(rest, tok);
}

NodePtr Node::Postfix(TokenPtr* rest, TokenPtr tok) {
  NodePtr node = Primary(&tok, tok);

  while (tok->Equal("[")) {
    // x[y] is short for *(x + y)
    TokenPtr node_name = tok;
    NodePtr idx = Expr(&tok, tok->next);
    tok = tok->SkipToken("]");
    NodePtr op = CreateAddNode(node_name, node, idx);
    node = CreateUnaryNode(ND_DEREF, node_name, op);
  }
  *rest = tok;
  return node;
}

// primary = "(" "{" stmt+ "}" ")"
//          |"(" expr ")" | "sizeof" unary | ident func-args? | str | num
NodePtr Node::Primary(TokenPtr* rest, TokenPtr tok) {
  // This is a GNU statement expression.
  if (tok->Equal("(") && tok->next->Equal("{")) {
    TokenPtr start = tok;
    NodePtr stmt = CompoundStmt(&tok, tok->next->next);
    *rest = tok->SkipToken(")");
    return CreateBlockNode(ND_STMT_EXPR, start, stmt->body);
  }
  if (tok->Equal("(")) {
    NodePtr node = Expr(&tok, tok->next);
    *rest = tok->SkipToken(")");
    return node;
  }

  if (tok->Equal("sizeof")) {
    NodePtr node = Unary(rest, tok->next);
    node->TypeInfer();
    long size = node->ty->size;
    return CreateConstNode(size, tok);
  }

  if (tok->kind == TK_IDENT) {
    if (tok->next->Equal("(")) {
      return Call(rest, tok);
    }
    *rest = tok->next;
    return CreateIdentNode(tok);
  }

  if (tok->kind == TK_STR) {
    ObjectPtr var = Object::CreateStringVar(tok->str_literal);
    *rest = tok->next;
    return CreateVarNode(var, tok);
  }

  if (tok->kind == TK_NUM) {
    NodePtr node = CreateConstNode(tok->val, tok);
    *rest = tok->next;
    return node;
  }

  tok->ErrorTok("expected an expression");
  return nullptr;
}

// function = ident "(" (assign ("," assign)*)? ")"
NodePtr Node::Call(TokenPtr* rest, TokenPtr tok) {
  TokenPtr start = tok;
  tok = tok->next->next;

  NodePtr args = std::make_shared<Node>(ND_END, tok);
  NodePtr cur = args;

  while (!tok->Equal(")")) {
    if (cur != args) {
      tok = tok->SkipToken(",");
    }
    cur->next = Assign(&tok, tok);
    cur = cur->next;
  }

  *rest = tok->SkipToken(")");
  return CreateCallNode(start, args->next);
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
      if (lhs->ty->kind == TY_ARRAY) {
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
      ty = var->ty;
      return;
    case ND_ADDR:
      if (lhs->ty->kind == TY_ARRAY) {
        ty = Type::CreatePointerType(lhs->ty->base);
      } else {
        ty = Type::CreatePointerType(lhs->ty);
      }
      return;
    case ND_DEREF:
      if (lhs->ty->base == nullptr) {
        ErrorTok("invalid pointer reference!");
      }
      ty = lhs->ty->base;
      return;
    case ND_STMT_EXPR:
      if(body != nullptr){
        NodePtr stmt = body;
        while(stmt->next != nullptr){
          stmt = stmt->next;
        }
        if(stmt->kind == ND_EXPR_STMT){
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

// Report an error based on tok
void Node::ErrorTok(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  name->ErrorTok(fmt, ap);
}
