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

#include "token.h"
#include "tools.h"
#include "type.h"
#include "var.h"

#include <cstring>

extern Var* locals;

Node::Node(NodeKind kind, Token* tok, Node* lhs, Node* rhs)
    : kind_(kind)
    , tok_(tok)
    , next_(nullptr)
    , lhs_(lhs)
    , rhs_(rhs)
    , cond_(nullptr)
    , then_(nullptr)
    , els_(nullptr)
    , body_(nullptr)
    , init_(nullptr)
    , inc_(nullptr)
    , call_(nullptr)
    , args_(nullptr)
    , val_(0)
    , var_()
    , ty_(nullptr) {
  if (!(kind_ == ND_ADD || kind_ == ND_SUB)) {
    return;
  }

  lhs_->TypeInfer();
  rhs_->TypeInfer();

  if (lhs_->ty_->IsInteger() && rhs_->ty_->IsInteger()) {
    ty_ = ty_int;
    return;
  }

  if (kind_ == ND_ADD) {
    if (lhs_->ty_->IsPointer() && rhs_->ty_->IsPointer()) {
      ErrorTok("Invalid Operands.");
    }
    // cononical 'num + ptr' to 'ptr + num'.
    if (!lhs_->ty_->IsPointer() && rhs_->ty_->IsPointer()) {
      Node* tmp = lhs_;
      lhs_      = rhs_;
      rhs_      = tmp;
    }

    // ptr + num
    rhs_ = new Node(ND_MUL, tok, rhs_, new Node(lhs_->ty_->base_->size_, tok));
    ty_  = lhs_->ty_;
    return;
  }

  if (kind_ == ND_SUB) {
    // ptr - ptr
    if (lhs_->ty_->IsPointer() && rhs_->ty_->IsPointer()) {
      kind_ = ND_DIV;
      // careful curisive call.
      Node* node = new Node(ND_SUB, tok);
      node->lhs_ = lhs_;
      node->rhs_ = rhs_;
      node->ty_  = ty_int;
      lhs_       = node;
      rhs_       = new Node(lhs->ty_->base_->size_, tok);
      return;
    }

    // ptr - num
    if (lhs_->ty_->IsPointer() && rhs_->ty_->IsInteger()) {
      rhs_ =
          new Node(ND_MUL, tok, rhs_, new Node(lhs_->ty_->base_->size_, tok));
      rhs_->TypeInfer();
      ty_ = lhs_->ty_;
      return;
    }
  }
}

Node* Node::Program(Token** rest, Token* tok) {
  tok = tok->SkipToken("{");
  return CompoundStmt(rest, tok);
}

// compound-stmt  = (declaration | stmt)* "}"
Node* Node::CompoundStmt(Token** rest, Token* tok) {
  Node  head = Node(ND_END, tok);
  Node* cur  = &head;

  while (!tok->Equal("}")) {
    if (tok->Equal("int")) {
      cur->next_ = Declaration(&tok, tok);
    } else {
      cur->next_ = Node::Stmt(&tok, tok);
    }
    cur = cur->next_;
    cur->TypeInfer();
  }
  Node* node  = new Node(ND_BLOCK, tok);
  node->body_ = head.next_;

  *rest = tok->next_;
  return node;
}

// declaration = declspec (
//                 declarator ( "=" expr)?
//                 ("," declarator ("=" expr)? ) * )? ";"
Node* Node::Declaration(Token** rest, Token* tok) {
  Type* ty_base = Declspec(&tok, tok);

  Node  head = Node(ND_END, tok);
  Node* cur  = &head;
  int   i    = 0;

  while (!tok->Equal(";")) {
    if (i++ > 0) {
      tok = tok->SkipToken(",");
    }

    Type* ty  = Declarator(&tok, tok, ty_base);
    Var*  var = new Var(ty->name_->GetIdent(), &locals, ty);

    if (!tok->Equal("=")) {
      continue;
    }

    Node* lhs  = new Node(var, tok);
    Node* rhs  = Node::Assign(&tok, tok->next_);
    Node* node = new Node(ND_ASSIGN, tok, lhs, rhs);
    cur->next_ = new Node(ND_EXPR_STMT, tok, node);
    cur        = cur->next_;
  }

  Node* node  = new Node(ND_BLOCK, tok);
  node->body_ = head.next_;
  *rest       = tok->next_;
  return node;
}

// declspec = "int"
Type* Node::Declspec(Token** rest, Token* tok) {
  *rest = tok->SkipToken("int");
  return ty_int;
}

// declarator = "*"* ident type-suffix
Type* Node::Declarator(Token** rest, Token* tok, Type* ty) {
  while (tok->Equal("*")) {
    ty  = new Type(TY_PRT, ty);
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

// type-suffix = "(" func-params | "[" num "]" type-suffix | ɛ
Type* Node::TypeSuffix(Token** rest, Token* tok, Type* ty) {
  if (tok->Equal("(")) {
    return FunctionParam(rest, tok->next_, ty);
  }
  if (tok->Equal("[")) {
    int len = tok->next_->GetNumber();
    tok     = tok->next_->next_->SkipToken("]");
    ty      = TypeSuffix(rest, tok, ty);
    return new Type(TY_ARRAY, ty, len);
  }

  *rest = tok;
  return ty;
}

// func-param = param ("," param) *
// param = declspec declarator
Type* Node::FunctionParam(Token** rest, Token* tok, Type* ty) {
  Type  head = Type();
  Type* cur  = &head;

  while (!tok->Equal(")")) {
    if (cur != &head) {
      tok = tok->SkipToken(",");
    }
    Type* base_ty = Declspec(&tok, tok);
    Type* ty      = Declarator(&tok, tok, base_ty);
    cur->next_    = new Type(ty);
    cur           = cur->next_;
  }

  ty          = new Type(TY_FUNC, ty);
  ty->params_ = head.next_;

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
    Node* node = new Node(ND_RETURN, tok, Expr(&tok, tok->next_));
    *rest      = tok->SkipToken(";");
    return node;
  }

  if (tok->Equal("if")) {
    Node* node  = new Node(ND_IF, tok);
    tok         = tok->next_;
    node->cond_ = Expr(&tok, tok->SkipToken("("));
    node->then_ = Stmt(&tok, tok->SkipToken(")"));
    if (tok->Equal("else")) {
      node->els_ = Stmt(&tok, tok->next_);
    }
    *rest = tok;
    return node;
  }

  if (tok->Equal("for")) {
    Node* node = new Node(ND_FOR, tok);
    tok        = tok->next_;
    tok        = tok->SkipToken("(");

    node->init_ = ExprStmt(&tok, tok);

    if (!tok->Equal(";")) {
      node->cond_ = Expr(&tok, tok);
    }
    tok = tok->SkipToken(";");

    if (!tok->Equal(")")) {
      node->inc_ = Expr(&tok, tok);
    }
    tok = tok->SkipToken(")");
    // then branch, not support {...}
    node->then_ = Stmt(rest, tok);

    return node;
  }

  if (tok->Equal("while")) {
    Node* node = new Node(ND_FOR, tok);
    tok        = tok->next_;
    tok        = tok->SkipToken("(");

    node->cond_ = Expr(&tok, tok);
    tok         = tok->SkipToken(")");

    node->then_ = Stmt(rest, tok);

    return node;
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
    return new Node(ND_BLOCK, tok);
  }

  Node* node = new Node(ND_EXPR_STMT, tok, Expr(&tok, tok));
  *rest      = tok->SkipToken(";");
  return node;
}

// expr = assign
Node* Node::Expr(Token** rest, Token* tok) {
  return Assign(rest, tok);
}

// assign = equality ("=" assign)?
Node* Node::Assign(Token** rest, Token* tok) {
  Node* node = Equality(&tok, tok);

  if (tok->Equal("=")) {
    return new Node(ND_ASSIGN, tok, node, Assign(rest, tok->next_));
  }
  *rest = tok;
  return node;
}

// equality = relational ("==" relational | "!=" relational)
Node* Node::Equality(Token** rest, Token* tok) {
  Node* node = Relational(&tok, tok);

  for (;;) {
    Token* start = tok;
    if (tok->Equal("==")) {
      node = new Node(ND_EQ, start, node, Relational(&tok, tok->next_));
      continue;
    }
    if (tok->Equal("!=")) {
      node = new Node(ND_NE, start, node, Relational(&tok, tok->next_));
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
    Token* start = tok;
    if (tok->Equal("<")) {
      node = new Node(ND_LT, start, node, Add(&tok, tok->next_));
      continue;
    }

    if (tok->Equal("<=")) {
      node = new Node(ND_LE, start, node, Add(&tok, tok->next_));
      continue;
    }

    if (tok->Equal(">")) {
      node = new Node(ND_LT, start, Add(&tok, tok->next_), node);
      continue;
    }

    if (tok->Equal(">=")) {
      node = new Node(ND_LE, start, Add(&tok, tok->next_), node);
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
    Token* start = tok;
    if (tok->Equal("+")) {
      node = new Node(ND_ADD, start, node, Mul(&tok, tok->next_));
      continue;
    }

    if (tok->Equal("-")) {
      node = new Node(ND_SUB, start, node, Mul(&tok, tok->next_));
      continue;
    }

    *rest = tok;
    return node;
  }
}

Node* Node::Mul(Token** rest, Token* tok) {
  Node* node = Unary(&tok, tok);

  for (;;) {
    Token* start = tok;
    if (tok->Equal("*")) {
      node = new Node(ND_MUL, start, node, Primary(&tok, tok->next_));
      continue;
    }

    if (tok->Equal("/")) {
      node = new Node(ND_DIV, start, node, Primary(&tok, tok->next_));
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
    return new Node(ND_NEG, tok, Unary(rest, tok->next_));
  }

  if (tok->Equal("&")) {
    return new Node(ND_ADDR, tok, Unary(rest, tok->next_));
  }

  if (tok->Equal("*")) {
    return new Node(ND_DEREF, tok, Unary(rest, tok->next_));
  }

  return Postfix(rest, tok);
}

Node* Node::Postfix(Token** rest, Token* tok) {
  Node* node = Primary(&tok, tok);

  while (tok->Equal("[")) {
    // x[y] is short for *(x + y)
    Token* start = tok;
    Node*  idx   = Expr(&tok, tok->next_);
    tok          = tok->SkipToken("]");
    node = new Node(ND_DEREF, start, new Node(ND_ADD, start, node, idx));
  }
  *rest = tok;
  return node;
}

// primary = "(" expr ")" | "sizeof" unary | ident | num
Node* Node::Primary(Token** rest, Token* tok) {
  if (tok->Equal("(")) {
    Node* node = Expr(&tok, tok->next_);
    *rest      = tok->SkipToken(")");
    return node;
  }

  if (tok->Equal("sizeof")) {
    Node* node = Unary(rest, tok->next_);
    node->TypeInfer();
    long size = node->ty_->size_;
    NodeFree(node);
    return new Node(size, tok);
  }

  if (tok->kind_ == TK_IDENT) {
    if (tok->next_->Equal("(")) {
      return Call(rest, tok);
    }
    Var* var = locals->Find(tok);
    if (var == nullptr) {
      tok->ErrorTok("undefined variable.");
    }
    *rest = tok->next_;
    return new Node(var, tok);
  }

  if (tok->kind_ == TK_NUM) {
    Node* node = new Node(tok->val_, tok);
    *rest      = tok->next_;
    return node;
  }

  tok->ErrorTok("expected an expression");
  return nullptr;
}

// function = ident "(" (assign ("," assign)*)? ")"
Node* Node::Call(Token** rest, Token* tok) {
  Token* start = tok;
  tok          = tok->next_->next_;

  Node  head = Node(ND_END, tok);
  Node* cur  = &head;

  while (!tok->Equal(")")) {
    if (cur != &head) {
      tok = tok->SkipToken(",");
    }
    cur->next_ = Assign(&tok, tok);
    cur        = cur->next_;
  }

  *rest = tok->SkipToken(")");

  Node* node  = new Node(ND_CALL, start);
  node->call_ = start->GetIdent();
  node->args_ = head.next_;
  return node;
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
  if (node == nullptr)
    return;

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
  case ND_NEG: ty_ = lhs_->ty_; return;
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
  case ND_CALL: ty_ = ty_int; return;
  case ND_VAR: ty_ = var_->ty_; return;
  case ND_ADDR:
    if (lhs_->ty_->kind_ == TY_ARRAY) {
      ty_ = new Type(TY_PRT, lhs_->ty_->base_);
    } else {
      ty_ = new Type(TY_PRT, lhs_->ty_);
    }
    return;
  case ND_DEREF:
    if (lhs_->ty_->base_ == nullptr) {
      ErrorTok("invalid pointer reference!");
    }
    ty_ = lhs_->ty_->base_;
    return;
  default: return;
  }
}

// Report an error based on tok
void Node::ErrorTok(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  tok_->ErrorTok(fmt, ap);
}
