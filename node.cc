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
    , val_(0)
    , var_()
    , ty_() {
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

    rhs_ = new Node(ND_MUL, tok, rhs_, new Node(8, tok));
    ty_ = lhs_->ty_;
    return ;
  }

  if (kind_ == ND_SUB) {
    // ptr - ptr 
    if(lhs_->ty_->IsPointer() && rhs_->ty_->IsPointer()){
      kind_ = ND_DIV;
      // careful curisive call.
      Node* node = new Node(ND_SUB, tok);
      node->lhs_ = lhs_;
      node->rhs_ = rhs_;
      node->ty_ = ty_int;
      lhs_ = node;
      rhs_ = new Node(8, tok);
      return ;
    }

    // ptr - num
    if(lhs_->ty_->IsPointer() && rhs_->ty_->IsInteger()){
      rhs_ = new Node(ND_MUL, tok, rhs_, new Node(8, tok));
      rhs_->TypeInfer();
      ty_ = lhs_->ty_;
      return ;
    }
  }
}

Node* Node::Program(Token* tok) {
  tok = tok->SkipToken("{");
  return CompoundStmt(&tok, tok);
}

// compound-stmt = stmt* "}"
Node* Node::CompoundStmt(Token** rest, Token* tok) {
  Node  head = Node(ND_END, tok);
  Node* cur  = &head;

  while (!tok->Equal("}")) {
    cur->next_ = Node::Stmt(&tok, tok);
    cur        = cur->next_;
  }
  Node* node = new Node(ND_BLOCK, tok, head.next_);
  *rest      = tok->next_;
  return node;
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

Node* Node::Assign(Token** rest, Token* tok) {
  Node* node = Equality(&tok, tok);

  if (tok->Equal("=")) {
    return new Node(ND_ASSIGN, tok, node, Assign(rest, tok->next_));
  }
  *rest = tok;
  return node;
}

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

  return Primary(rest, tok);
}

Node* Node::Primary(Token** rest, Token* tok) {
  if (tok->Equal("(")) {
    Node* node = Expr(&tok, tok->next_);
    *rest      = tok->SkipToken(")");
    return node;
  }

  if (tok->kind_ == TK_IDENT) {
    Var* var = locals->Find(tok);
    if (var == nullptr) {
      var    = new Var(strndup(tok->str_, tok->strlen_), locals);
      locals = var;
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
// TODO: rename this function to AST_Tree free
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
  if(node->kind_ == ND_ADDR)
    delete node->ty_;
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
  case ND_ASSIGN: ty_ = lhs_->ty_; return;
  case ND_EQ:
  case ND_NE:
  case ND_LE:
  case ND_LT:
  case ND_VAR:
  case ND_NUM: ty_ = ty_int; return;
  case ND_ADDR: ty_ = new Type(TY_PRT, lhs_->ty_); return;
  case ND_DEREF:
    if (lhs_->ty_->kind_ == TY_PRT) {
      ty_ = lhs_->ty_->base_;
    } else {
      ty_ = ty_int;
    }
    return;
  default: ErrorTok("expected an expression"); return;
  }
}

// Report an error based on tok
void Node::ErrorTok(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  tok_->ErrorTok(fmt, ap);
}
