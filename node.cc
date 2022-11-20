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

Node* Node::Program(Token** tok) {
  Node  head = Node();
  Node* cur  = &head;

  while (!Token::IsEof(*tok)) {
    cur->next = Stmt(tok);
    cur       = cur->next;
  }
  return head.next;
}

Node* Node::Stmt(Token** tok) {
  if (Token::Consume(tok, "return")) {
    Node* node = new Node(ND_RETURN, Expr(tok));
    Token::Expect(tok, ";");
    return node;
  }
  Node* node = Expr(tok);
  Token::Expect(tok, ";");
  return node;
}

Node* Node::Expr(Token** tok) {
  return Equality(tok);
}

Node* Node::Equality(Token** tok) {
  Node* node = Relational(tok);

  for (;;) {
    if (Token::Consume(tok, "==")) {
      node = new Node(ND_EQ, node, Relational(tok));
    } else if (Token::Consume(tok, "!=")) {
      node = new Node(ND_NE, node, Relational(tok));
    } else {
      return node;
    }
  }
}

Node* Node::Relational(Token** tok) {
  Node* node = Add(tok);

  for (;;) {
    if (Token::Consume(tok, "<")) {
      node = new Node(ND_LT, node, Add(tok));
    } else if (Token::Consume(tok, "<=")) {
      node = new Node(ND_LE, node, Add(tok));
    } else if (Token::Consume(tok, ">")) {
      node = new Node(ND_LT, Add(tok), node);
    } else if (Token::Consume(tok, ">=")) {
      node = new Node(ND_LE, Add(tok), node);
    } else {
      return node;
    }
  }
}

Node* Node::Add(Token** tok) {
  Node* node = Mul(tok);

  for (;;) {
    if (Token::Consume(tok, "+")) {
      node = new Node(ND_ADD, node, Mul(tok));
    } else if (Token::Consume(tok, "-")) {
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
    if (Token::Consume(tok, "*")) {
      node = new Node(ND_MUL, node, Primary(tok));
    } else if (Token::Consume(tok, "/")) {
      node = new Node(ND_DIV, node, Primary(tok));
    } else {
      return node;
    }
  }
}

Node* Node::Unary(Token** tok) {
  if (Token::Consume(tok, "+")) {
    return Unary(tok);
  }
  if (Token::Consume(tok, "-")) {
    return new Node(ND_SUB, new Node(0), Unary(tok));
  }
  return Primary(tok);
}

Node* Node::Primary(Token** tok) {
  if (Token::Consume(tok, "(")) {
    Node* node = Expr(tok);
    Token::Expect(tok, ")");
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
