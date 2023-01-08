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

#include <cstring>

#include "object.h"
#include "token.h"
#include "tools.h"
#include "type.h"

extern Object* locals;

Node* Node::CreateConstNode(long val, Token* tok) {
  Node* node = new Node(ND_NUM, tok);
  node->val_ = val;
  return node;
}

Node* Node::CreateVarNode(Object* var, Token* tok) {
  Node* node = new Node(ND_VAR, tok);
  node->var_ = var;
  return node;
}

Node* Node::CreateIdentNode(Token* tok) {
  Object* local_var = tok->FindLocalVar();
  Object* global_var = tok->FindGlobalVar();
  if (global_var == nullptr && local_var == nullptr) {
    tok->ErrorTok("undefined variable.");
  }
  if (local_var) {
    return CreateVarNode(local_var, tok);
  }
  return CreateVarNode(global_var, tok);
}

Node* Node::CreateCallNode(Token* call_name, Node* args) {
  Node* call_node = new Node(ND_CALL, call_name);
  call_node->call_ = call_name->GetIdent();
  call_node->args_ = args;
  return call_node;
}

Node* Node::CreateUnaryNode(NodeKind kind, Token* node_name, Node* op) {
  Node* res = new Node(kind, node_name);
  res->lhs_ = op;
  return res;
}

Node* Node::CreateAddNode(Token* node_name, Node* op_left, Node* op_right) {
  op_left->TypeInfer();
  op_right->TypeInfer();
  if (!op_left->IsPointerNode() && !op_right->IsPointerNode()) {
    Node* res = CreateBinaryNode(ND_ADD, node_name, op_left, op_right);
    res->ty_ = ty_int.get();
    return res;
  }
  // ptr + ptr
  if (op_left->IsPointerNode() && op_right->IsPointerNode()) {
    node_name->ErrorTok("Invalid Operands.");
  }
  // cononical 'num + ptr' to 'ptr + num'.
  if (!op_left->IsPointerNode() && op_right->IsPointerNode()) {
    Node* tmp = op_left;
    op_left = op_right;
    op_right = tmp;
  }
  // ptr + num
  Node* factor = CreateConstNode(op_left->ty_->base_->size_, node_name);
  Node* real_num = CreateBinaryNode(ND_MUL, node_name, op_right, factor);
  Node* res = CreateBinaryNode(ND_ADD, node_name, op_left, real_num);
  return res;
}

Node* Node::CreateSubNode(Token* node_name, Node* op_left, Node* op_right) {
  op_left->TypeInfer();
  op_right->TypeInfer();
  // num - num
  if (!op_left->IsPointerNode() && !op_right->IsPointerNode()) {
    Node* res = CreateBinaryNode(ND_SUB, node_name, op_left, op_right);
    res->ty_ = ty_int.get();
    return res;
  }
  // ptr - ptr
  if (op_left->IsPointerNode() && op_right->IsPointerNode()) {
    // careful curisive call.
    Node* sub = CreateBinaryNode(ND_SUB, node_name, op_left, op_right);
    Node* factor = CreateConstNode(op_left->ty_->base_->size_, node_name);
    Node* res = CreateBinaryNode(ND_DIV, node_name, sub, factor);
    res->ty_ = ty_int.get();
    return res;
  }
  if (!op_left->IsPointerNode() && op_right->IsPointerNode()) {
    node_name->ErrorTok("Invalid Operands.");
  }
  // ptr - num
  Node* factor = CreateConstNode(op_left->ty_->base_->size_, node_name);
  Node* real_num = CreateBinaryNode(ND_MUL, node_name, op_right, factor);
  Node* res = CreateBinaryNode(ND_SUB, node_name, op_left, real_num);
  return res;
}

Node* Node::CreateBinaryNode(NodeKind kind, Token* node_name, Node* op_left, Node* op_right) {
  Node* res = new Node(kind, node_name);
  res->lhs_ = op_left;
  res->rhs_ = op_right;
  return res;
}

Node* Node::CreateIFNode(Token* node_name, Node* cond, Node* then, Node* els) {
  Node* res = new Node(ND_IF, node_name);
  res->cond_ = cond;
  res->then_ = then;
  res->els_ = els;
  return res;
}

Node* Node::CreateForNode(Token* node_name, Node* init, Node* cond, Node* inc, Node* then) {
  Node* res = new Node(ND_FOR, node_name);
  res->init_ = init;
  res->cond_ = cond;
  res->inc_ = inc;
  res->then_ = then;
  return res;
}

Node* Node::CreateBlockNode(Token* node_name, Node* body) {
  Node* res = new Node(ND_BLOCK, node_name);
  res->body_ = body;
  return res;
}

Node* Node::Program(Token** rest, Token* tok) {
  tok = tok->SkipToken("{");
  return CompoundStmt(rest, tok);
}

// compound-stmt  = (declaration | stmt)* "}"
Node* Node::CompoundStmt(Token** rest, Token* tok) {
  Node sub_expr;
  Node* cur = &sub_expr;

  while (!tok->Equal("}")) {
    if (tok->IsTypename()) {
      cur->next_ = Declaration(&tok, tok);
    } else {
      cur->next_ = Node::Stmt(&tok, tok);
    }
    cur = cur->next_;
    cur->TypeInfer();
  }

  *rest = tok->next_;
  return CreateBlockNode(tok, sub_expr.next_);
  ;
}

// declaration = declspec (
//                 declarator ( "=" expr)?
//                 ("," declarator ("=" expr)? ) * )? ";"
Node* Node::Declaration(Token** rest, Token* tok) {
  Type* ty_base = Declspec(&tok, tok);

  Node decl_expr = Node();
  Node* cur = &decl_expr;
  int i = 0;

  while (!tok->Equal(";")) {
    if (i++ > 0) {
      tok = tok->SkipToken(",");
    }
    Type* ty = Declarator(&tok, tok, ty_base);
    Object* var = Object::CreateLocalVar(ty->name_->GetIdent(), ty, &locals);
    if (!tok->Equal("=")) {
      continue;
    }
    Node* lhs = CreateVarNode(var, tok);
    Node* rhs = Assign(&tok, tok->next_);
    Node* assgin_node = CreateBinaryNode(ND_ASSIGN, tok, lhs, rhs);
    cur = cur->next_ = CreateUnaryNode(ND_EXPR_STMT, tok, assgin_node);
  }

  *rest = tok->next_;
  return CreateBlockNode(tok, decl_expr.next_);
  ;
}

// declspec = "char" | "int"
Type* Node::Declspec(Token** rest, Token* tok) {
  if (tok->Equal("char")) {
    *rest = tok->SkipToken("char");
    return ty_char.get();
  }

  *rest = tok->SkipToken("int");
  return ty_int.get();
}

// declarator = "*"* ident type-suffix
Type* Node::Declarator(Token** rest, Token* tok, Type* ty) {
  while (tok->Equal("*")) {
    ty = Type::CreatePointerType(ty);
    tok = tok->next_;
  }
  *rest = tok;

  if (tok->kind_ != TK_IDENT) {
    tok->ErrorTok("expected a variable name.");
  }
  ty = TypeSuffix(rest, tok->next_, ty);
  ty->name_ = tok;
  return ty;
}

// type-suffix = "(" func-params ")" | "[" num "]" type-suffix | ɛ
Type* Node::TypeSuffix(Token** rest, Token* tok, Type* ty) {
  if (tok->Equal("(")) {
    return FunctionParam(rest, tok->next_, ty);
  }
  if (tok->Equal("[")) {
    int len = tok->next_->GetNumber();
    tok = tok->next_->next_->SkipToken("]");
    ty = TypeSuffix(rest, tok, ty);
    return Type::CreateArrayType(ty, len);
  }

  *rest = tok;
  return ty;
}

// func-param = param ("," param) *
// param = declspec declarator
Type* Node::FunctionParam(Token** rest, Token* tok, Type* ty) {
  Type params;
  Type* cur = &params;

  while (!tok->Equal(")")) {
    if (cur != &params) {
      tok = tok->SkipToken(",");
    }
    Type* base_ty = Declspec(&tok, tok);
    Type* ty = Declarator(&tok, tok, base_ty);
    cur->next_ = new Type(*ty);
    cur = cur->next_;
  }
  ty = Type::CreateFunctionType(ty, params.next_);

  *rest = tok->next_;
  return ty;
}

// stmt = "return" expr ";" |
// "if" "(" expr ")" stmt ("else" stmt)? |
// "for" "(" expr-stmt expr? ";" expr? ")" stmt |
// "{" compuound-stmt |
// expr-stmt
Node* Node::Stmt(Token** rest, Token* tok) {
  if (tok->Equal("return")) {
    Node* node = CreateUnaryNode(ND_RETURN, tok, Expr(&tok, tok->next_));
    *rest = tok->SkipToken(";");
    return node;
  }

  if (tok->Equal("if")) {
    Token* node_name = tok;
    tok = tok->next_;
    Node* cond = Expr(&tok, tok->SkipToken("("));
    Node* then = Stmt(&tok, tok->SkipToken(")"));
    Node* els = nullptr;
    if (tok->Equal("else")) {
      els = Stmt(&tok, tok->next_);
    }
    *rest = tok;
    return CreateIFNode(node_name, cond, then, els);
  }

  if (tok->Equal("for")) {
    Token* node_name = tok;
    tok = tok->next_->SkipToken("(");

    Node* init = ExprStmt(&tok, tok);
    Node* cond = nullptr;
    Node* inc = nullptr;

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
    Token* node_name = tok;
    tok = tok->next_->SkipToken("(");

    Node* cond = Expr(&tok, tok);
    tok = tok->SkipToken(")");
    Node* then = Stmt(rest, tok);
    return CreateForNode(node_name, nullptr, cond, nullptr, then);
  }

  if (tok->Equal("{")) {
    return CompoundStmt(rest, tok->next_);
  }

  return ExprStmt(rest, tok);
}

// expr-stmt = expr ";"
Node* Node::ExprStmt(Token** rest, Token* tok) {
  if (tok->Equal(";")) {
    *rest = tok->next_;
    return CreateBlockNode(tok, nullptr);
  }
  Node* node = CreateUnaryNode(ND_EXPR_STMT, tok, Expr(&tok, tok));
  *rest = tok->SkipToken(";");
  return node;
}

// expr = assign
Node* Node::Expr(Token** rest, Token* tok) { return Assign(rest, tok); }

// assign = equality ("=" assign)?
Node* Node::Assign(Token** rest, Token* tok) {
  Node* node = Equality(&tok, tok);
  if (tok->Equal("=")) {
    return CreateBinaryNode(ND_ASSIGN, tok, node, Assign(rest, tok->next_));
  }
  *rest = tok;
  return node;
}

// equality = relational ("==" relational | "!=" relational)
Node* Node::Equality(Token** rest, Token* tok) {
  Node* node = Relational(&tok, tok);

  for (;;) {
    Token* node_name = tok;
    if (tok->Equal("==")) {
      node = CreateBinaryNode(ND_EQ, node_name, node, Relational(&tok, tok->next_));
      continue;
    }
    if (tok->Equal("!=")) {
      node = CreateBinaryNode(ND_NE, node_name, node, Relational(&tok, tok->next_));
      continue;
    }

    *rest = tok;
    return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)
Node* Node::Relational(Token** rest, Token* tok) {
  Node* node = Add(&tok, tok);

  for (;;) {
    Token* node_name = tok;
    if (tok->Equal("<")) {
      node = CreateBinaryNode(ND_LT, node_name, node, Add(&tok, tok->next_));
      continue;
    }
    if (tok->Equal("<=")) {
      node = CreateBinaryNode(ND_LE, node_name, node, Add(&tok, tok->next_));
      continue;
    }
    if (tok->Equal(">")) {
      node = CreateBinaryNode(ND_LT, node_name, Add(&tok, tok->next_), node);
      continue;
    }
    if (tok->Equal(">=")) {
      node = CreateBinaryNode(ND_LE, node_name, Add(&tok, tok->next_), node);
      continue;
    }
    *rest = tok;
    return node;
  }
}

// add = mul ("+"mul | "-" mul)
Node* Node::Add(Token** rest, Token* tok) {
  Node* node = Mul(&tok, tok);

  for (;;) {
    Token* node_name = tok;
    if (tok->Equal("+")) {
      node = CreateAddNode(node_name, node, Mul(&tok, tok->next_));
      continue;
    }
    if (tok->Equal("-")) {
      node = CreateSubNode(node_name, node, Mul(&tok, tok->next_));
      continue;
    }
    *rest = tok;
    return node;
  }
}

Node* Node::Mul(Token** rest, Token* tok) {
  Node* node = Unary(&tok, tok);

  for (;;) {
    Token* node_name = tok;
    if (tok->Equal("*")) {
      node = CreateBinaryNode(ND_MUL, node_name, node, Primary(&tok, tok->next_));
      continue;
    }
    if (tok->Equal("/")) {
      node = CreateBinaryNode(ND_DIV, node_name, node, Primary(&tok, tok->next_));
      continue;
    }
    *rest = tok;
    return node;
  }
}

// unary = ("+" | "-" | "*" | "&") ? unary | primary
Node* Node::Unary(Token** rest, Token* tok) {
  if (tok->Equal("+")) {
    return Unary(rest, tok->next_);
  }
  if (tok->Equal("-")) {
    return CreateUnaryNode(ND_NEG, tok, Unary(rest, tok->next_));
  }
  if (tok->Equal("&")) {
    return CreateUnaryNode(ND_ADDR, tok, Unary(rest, tok->next_));
  }
  if (tok->Equal("*")) {
    return CreateUnaryNode(ND_DEREF, tok, Unary(rest, tok->next_));
  }
  return Postfix(rest, tok);
}

Node* Node::Postfix(Token** rest, Token* tok) {
  Node* node = Primary(&tok, tok);

  while (tok->Equal("[")) {
    // x[y] is short for *(x + y)
    Token* node_name = tok;
    Node* idx = Expr(&tok, tok->next_);
    tok = tok->SkipToken("]");
    Node* op = CreateAddNode(node_name, node, idx);
    node = CreateUnaryNode(ND_DEREF, node_name, op);
  }
  *rest = tok;
  return node;
}

// primary = "(" expr ")" | "sizeof" unary | ident | str | num
Node* Node::Primary(Token** rest, Token* tok) {
  if (tok->Equal("(")) {
    Node* node = Expr(&tok, tok->next_);
    *rest = tok->SkipToken(")");
    return node;
  }

  if (tok->Equal("sizeof")) {
    Node* node = Unary(rest, tok->next_);
    node->TypeInfer();
    long size = node->ty_->size_;
    NodeFree(node);
    return CreateConstNode(size, tok);
  }

  if (tok->kind_ == TK_IDENT) {
    if (tok->next_->Equal("(")) {
      return Call(rest, tok);
    }
    *rest = tok->next_;
    return CreateIdentNode(tok);
  }

  if (tok->kind_ == TK_STR) {
    Object* var = Object::CreateStringVar(tok->str_literal_);
    *rest = tok->next_;
    return CreateVarNode(var, tok);
  }

  if (tok->kind_ == TK_NUM) {
    Node* node = CreateConstNode(tok->val_, tok);
    *rest = tok->next_;
    return node;
  }

  tok->ErrorTok("expected an expression");
  return nullptr;
}

// function = ident "(" (assign ("," assign)*)? ")"
Node* Node::Call(Token** rest, Token* tok) {
  Token* start = tok;
  tok = tok->next_->next_;

  Node args;
  Node* cur = &args;

  while (!tok->Equal(")")) {
    if (cur != &args) {
      tok = tok->SkipToken(",");
    }
    cur->next_ = Assign(&tok, tok);
    cur = cur->next_;
  }

  *rest = tok->SkipToken(")");
  return CreateCallNode(start, args.next_);
}

void Node::NodeListFree(Node* node) {
  Node* cur = node;
  while (cur != nullptr) {
    node = node->next_;
    if (cur->kind_ == ND_IF) {
      NodeFree(cur->cond_);
      NodeListFree(cur->then_);
      NodeListFree(cur->els_);
    }
    if (cur->kind_ == ND_FOR) {
      NodeFree(cur->init_);
      NodeFree(cur->cond_);
      NodeFree(cur->inc_);
      NodeListFree(cur->then_);
    }
    if (cur->kind_ == ND_BLOCK) {
      NodeListFree(cur->body_);
    }
    NodeFree(cur);
    cur = node;
  }
}

// post-order for node delete
void Node::NodeFree(Node* node) {
  if (node == nullptr) return;

  if (node->lhs_ != nullptr) {
    NodeFree(node->lhs_);
    node->lhs_ = nullptr;
  }
  if (node->rhs_ != nullptr) {
    NodeFree(node->rhs_);
    node->rhs_ = nullptr;
  }
  if (node->kind_ == ND_ADDR) {
    delete node->ty_;
  }
  if (node->kind_ == ND_VAR) {
    // all of var will create a list and is freed in Funciton class.
    ;
  }
  if (node->kind_ == ND_CALL) {
    free(node->call_);
    NodeListFree(node->args_);
  }
  delete node;
  return;
}

void Node::TypeInfer() {
  if (this->ty_ != nullptr) {
    return;
  }

  if (lhs_ != nullptr) {
    lhs_->TypeInfer();
  }
  if (rhs_ != nullptr) {
    rhs_->TypeInfer();
  }
  if (cond_ != nullptr) {
    cond_->TypeInfer();
  }
  if (then_ != nullptr) {
    then_->TypeInfer();
  }
  if (els_ != nullptr) {
    els_->TypeInfer();
  }
  if (init_ != nullptr) {
    init_->TypeInfer();
  }
  if (inc_ != nullptr) {
    inc_->TypeInfer();
  }

  for (Node* n = body_; n != nullptr; n = n->next_) {
    n->TypeInfer();
  }

  switch (kind_) {
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_NEG:
      ty_ = lhs_->ty_;
      return;
    case ND_ASSIGN:
      if (lhs_->ty_->kind_ == TY_ARRAY) {
        lhs_->tok_->ErrorTok("not an lvalue");
      }
      ty_ = lhs_->ty_;
      return;
    case ND_EQ:
    case ND_NE:
    case ND_LE:
    case ND_LT:
    case ND_NUM:
    case ND_CALL:
      ty_ = ty_int.get();
      return;
    case ND_VAR:
      ty_ = var_->ty_;
      return;
    case ND_ADDR:
      if (lhs_->ty_->kind_ == TY_ARRAY) {
        ty_ = Type::CreatePointerType(lhs_->ty_->base_);
      } else {
        ty_ = Type::CreatePointerType(lhs_->ty_);
      }
      return;
    case ND_DEREF:
      if (lhs_->ty_->base_ == nullptr) {
        ErrorTok("invalid pointer reference!");
      }
      ty_ = lhs_->ty_->base_;
      return;
    default:
      return;
  }
}

bool Node::IsPointerNode() { return ty_->IsPointer(); }

// Report an error based on tok
void Node::ErrorTok(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  tok_->ErrorTok(fmt, ap);
}
