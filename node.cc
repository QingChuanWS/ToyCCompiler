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

Node* Node::Expr(Token** tok) {
  Node* node = Mul(tok);

  for (;;) {
    if (Token::Consume(tok, '+')) {
      node = new Node(ND_ADD, node, Mul(tok));
    } else if (Token::Consume(tok, '-')) {
      node = new Node(ND_SUB, node, Mul(tok));
    } else {
      return node;
    }
  }
  return node;
}

Node* Node::Mul(Token** tok) {
  Node* node = Unary(tok);

  for (;;) {
    if (Token::Consume(tok, '*')) {
      node = new Node(ND_MUL, node, Primary(tok));
    } else if (Token::Consume(tok, '/')) {
      node = new Node(ND_DIV, node, Primary(tok));
    } else {
      return node;
    }
  }
}

Node* Node::Unary(Token **tok){
  if(Token::Consume(tok, '+')){
    return Unary(tok);
  }
  if(Token::Consume(tok, '-')){
    return new Node(ND_SUB, new Node(0), Unary(tok));
  }
  return Primary(tok);
}

Node* Node::Primary(Token** tok) {
  if (Token::Consume(tok, '(')) {
    Node* node = Expr(tok);
    Token::Expect(tok, ')');
    return node;
  }
  return new Node(Token::ExpectNumber(tok));
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
  delete node;
  return;
}

// post-order for code-gen
void Node::CodeGen(Node* node) {
  if (node->kind_ == ND_NUM) {
    ASM_GEN("  push ", node->val_);
    return;
  }

  CodeGen(node->lhs_);
  CodeGen(node->rhs_);

  ASM_GEN("  pop rdi");
  ASM_GEN("  pop rax");

  switch (node->kind_) {
  case ND_ADD: ASM_GEN("  add rax, rdi"); break;
  case ND_SUB: ASM_GEN("  sub rax, rdi"); break;
  case ND_MUL: ASM_GEN("  imul rax, rdi"); break;
  case ND_DIV:
    ASM_GEN("  cqo");
    ASM_GEN("  idiv rdi");
    break;
  default: Error("error node type !"); break;
  }

  ASM_GEN("  push rax");
}
