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
#include "var.h"

#include <cstring>

extern Var* locals;

Node* Node::Program(Token* tok) {
  tok = tok->SkipToken("{");
  return CompoundStmt(&tok, tok);
}

// compound-stmt = stmt* "}"
Node* Node::CompoundStmt(Token** rest, Token* tok) {
  Node  head = Node(ND_END);
  Node* cur  = &head;

  while (!tok->Equal("}")) {
    cur->next_ = Node::Stmt(&tok, tok);
    cur        = cur->next_;
  }
  Node* node = new Node(ND_BLOCK, head.next_);
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
    Node* node = new Node(ND_RETURN, Expr(&tok, tok->next_));
    *rest      = tok->SkipToken(";");
    return node;
  }

  if (tok->Equal("if")) {
    Node* node  = new Node(ND_IF);
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
    Node* node = new Node(ND_FOR);
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
    Node* node = new Node(ND_FOR);
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
    return new Node(ND_BLOCK);
  }

  Node* node = new Node(ND_EXPR_STMT, Expr(&tok, tok));
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
    node = new Node(ND_ASSIGN, node, Assign(&tok, tok->next_));
  }
  *rest = tok;
  return node;
}

Node* Node::Equality(Token** rest, Token* tok) {
  Node* node = Relational(&tok, tok);

  for (;;) {
    if (tok->Equal("==")) {
      node = new Node(ND_EQ, node, Relational(&tok, tok->next_));
      continue;
    }
    if (tok->Equal("!=")) {
      node = new Node(ND_NE, node, Relational(&tok, tok->next_));
      continue;
    }

    *rest = tok;
    return node;
  }
}

Node* Node::Relational(Token** rest, Token* tok) {
  Node* node = Add(&tok, tok);

  for (;;) {
    if (tok->Equal("<")) {
      node = new Node(ND_LT, node, Add(&tok, tok->next_));
      continue;
    }

    if (tok->Equal("<=")) {
      node = new Node(ND_LE, node, Add(&tok, tok->next_));
      continue;
    }

    if (tok->Equal(">")) {
      node = new Node(ND_LT, Add(&tok, tok->next_), node);
      continue;
    }

    if (tok->Equal(">=")) {
      node = new Node(ND_LE, Add(&tok, tok->next_), node);
      continue;
    }

    *rest = tok;
    return node;
  }
}

Node* Node::Add(Token** rest, Token* tok) {
  Node* node = Mul(&tok, tok);

  for (;;) {
    if (tok->Equal("+")) {
      node = new Node(ND_ADD, node, Mul(&tok, tok->next_));
      continue;
    }

    if (tok->Equal("-")) {
      node = new Node(ND_SUB, node, Mul(&tok, tok->next_));
      continue;
    }

    *rest = tok;
    return node;
  }
}

Node* Node::Mul(Token** rest, Token* tok) {
  Node* node = Unary(&tok, tok);

  for (;;) {
    if (tok->Equal("*")) {
      node = new Node(ND_MUL, node, Primary(&tok, tok->next_));
      continue;
    }

    if (tok->Equal("/")) {
      node = new Node(ND_DIV, node, Primary(&tok, tok->next_));
      continue;
    }

    *rest = tok;
    return node;
  }
}

Node* Node::Unary(Token** rest, Token* tok) {
  if (tok->Equal("+")) {
    return Unary(rest, tok->next_);
  }

  if (tok->Equal("-")) {
    return new Node(ND_SUB, new Node((long)0), Unary(rest, tok->next_));
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
    return new Node(var);
  }

  if (tok->kind_ == TK_NUM) {
    Node* node = new Node(tok->val_);
    *rest      = tok->next_;
    return node;
  }

  Token::ErrorTok(Token::prg_, tok, "expected an expression");
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
  delete node;
  return;
}
