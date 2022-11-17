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

#include <cassert>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <string>

#define DEBUG(expr) assert(expr)

enum Tokenkind {
  TK_RESERVED,
  TK_NUM,
  TK_EOF,
};

void Error(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(0);
}

// Reports an error location and exit.
void ErrorAt(char* prg, char* loc, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - prg;
  fprintf(stderr, "%s\n", prg);
  fprintf(stderr, "%*s", pos, "");   // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void LOG(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}

class Token {
 public:
  Token()
      : kind_(TK_EOF)
      , next_(nullptr)
      , val_(0)
      , str_(nullptr) {}

  Token(Tokenkind kind, char* str)
      : kind_(kind)
      , str_(str)
      , val_(0)
      , next_(nullptr) {}

  Token(Tokenkind kind, Token* tok, char* str)
      : kind_(kind)
      , str_(str)
      , val_(0)
      , next_(nullptr) {
    tok->next_ = this;
  }

  static Token* Tokenize(Token& head, char* prg) {
    prg_       = prg;
    Token* cur = &head;
    char*  p   = prg_;
    while (*p != '\0') {
      if (std::isdigit(*p)) {
        cur->next_ = new Token(TK_NUM, p);
        cur        = cur->next_;
        cur->val_  = strtol(p, &p, 10);
        continue;
      }

      if (std::ispunct(*p)) {
        cur->next_ = new Token(TK_RESERVED, p);
        cur        = cur->next_;
        p++;
        continue;
      }

      if (std::isspace(*p)) {
        p++;
        continue;
      }

      ErrorAt(prg, p, "expect a number.");
    }

    cur->next_ = new Token(TK_EOF, p);
    return head.next_;
  }

  static void TokenFree(Token& head) {
    Token* cur = head.next_;
    while (cur != nullptr) {
      head.next_ = cur->next_;
      delete cur;
      cur = head.next_;
    }
  }

  static bool Consume(Token** tok, char op) {
    if ((*tok)->kind_ != TK_RESERVED || (*tok)->str_[0] != op) {
      return false;
    }
    NextToken(tok);
    return true;
  }

  static void Expect(Token** tok, char op) {
    if ((*tok)->kind_ != TK_RESERVED || (*tok)->str_[0] != op) {
      ErrorAt(Token::prg_, (*tok)->str_, "expect '%c'", op);
    }
    NextToken(tok);
  }

  static long ExpectNumber(Token** tok) {
    if ((*tok)->kind_ != TK_NUM) {
      ErrorAt(prg_, (*tok)->str_, "expect a number");
    }
    long val = (*tok)->val_;
    NextToken(tok);
    return val;
  }

  static bool IsEof(Token* tok) { return tok->kind_ == TK_EOF; }

 private:
  static void NextToken(Token** tok) { *tok = (*tok)->next_; }

  static char* prg_;

  Tokenkind kind_;
  long      val_;
  char*     str_;
  Token*    next_;
};

char* Token::prg_ = nullptr;

// Parse

enum NodeKind {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_NUM,
};

class Node {
 public:
  Node(NodeKind kind)
      : kind_(kind)
      , lhs_(nullptr)
      , rhs_(nullptr)
      , val_(0) {}
  Node(NodeKind kind, Node* lhs, Node* rhs)
      : kind_(kind)
      , lhs_(lhs)
      , rhs_(rhs)
      , val_(0) {}
  Node(int val)
      : kind_(ND_NUM)
      , lhs_(nullptr)
      , rhs_(nullptr)
      , val_(val) {}

  static Node* expr(Token** tok) {
    Node* node = mul(tok);

    for (;;) {
      if (Token::Consume(tok, '+')) {
        node = new Node(ND_ADD, node, mul(tok));
      } else if (Token::Consume(tok, '-')) {
        node = new Node(ND_SUB, node, mul(tok));
      } else {
        return node;
      }
    }
    return node;
  }

  static Node* mul(Token** tok) {
    Node* node = primary(tok);

    for (;;) {
      if (Token::Consume(tok, '*')) {
        node = new Node(ND_MUL, node, primary(tok));
      } else if (Token::Consume(tok, '/')) {
        node = new Node(ND_DIV, node, primary(tok));
      } else {
        return node;
      }
    }
  }

  static Node* primary(Token** tok) {
    if (Token::Consume(tok, '(')) {
      Node* node = expr(tok);
      Token::Expect(tok, ')');
      return node;
    }
    return new Node(Token::ExpectNumber(tok));
  }

  // post-order for node delete
  static void NodeFree(Node* node) {
    if(node == nullptr)
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
  static void CodeGen(Node* node) {
    if (node->kind_ == ND_NUM) {
      std::cout << "  push " << node->val_ << std::endl;
      return;
    }

    CodeGen(node->lhs_);
    CodeGen(node->rhs_);

    std::cout << "  pop rdi" << std::endl;
    std::cout << "  pop rax" << std::endl;

    switch (node->kind_) {
    case ND_ADD: std::cout << "  add rax, rdi" << std::endl; break;
    case ND_SUB: std::cout << "  sub rax, rdi" << std::endl; break;
    case ND_MUL: std::cout << "  imul rax, rdi" << std::endl; break;
    case ND_DIV:
      std::cout << "  cqo" << std::endl;
      std::cout << "  idiv rdi" << std::endl;
      break;
    default: Error("error node type !"); break;
    }

    std::cout << "  push rax" << std::endl;
  }

 private:
  NodeKind kind_;
  Node*    lhs_;
  Node*    rhs_;
  long     val_;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
    return 1;
  }

  Token  head;
  Token* cur  = Token::Tokenize(head, argv[1]);
  Node*  node = Node::expr(&cur);

  std::cout << ".intel_syntax noprefix" << std::endl;
  std::cout << ".global main" << std::endl;
  std::cout << "main:" << std::endl;

  Node::CodeGen(node);

  printf("  pop rax\n");
  printf("  ret\n");

  Node::NodeFree(node);
  Token::TokenFree(head);

  return 0;
}
