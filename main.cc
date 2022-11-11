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

void Error(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(0);
}

// Reports an error location and exit.
void ErrorAt(char *prg, char *loc, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - prg;
  fprintf(stderr, "%s\n", prg);
  fprintf(stderr, "%*s", pos, ""); // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void LOG(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}

class Token {
public:
  Token() : kind_(TK_EOF), next_(nullptr), val_(0), str_(nullptr) {}

  Token(Tokenkind kind, char *str)
      : kind_(kind), str_(str), val_(0), next_(nullptr) {}

  static Token *Tokenize(Token &head, char *prg) {
    prg_ = prg;
    Token *cur = &head;
    char *p = prg_;
    while (*p != '\0') {
      if (std::isdigit(*p)) {
        cur->next_ = new Token(TK_NUM, p);
        cur = cur->next_;
        cur->val_ = strtol(p, &p, 10);
        continue;
      }

      if (*p == '+' || *p == '-') {
        cur->next_ = new Token(TK_RESERVED, p);
        cur = cur->next_;
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

  static void TokenizeFree(Token &head) {
    Token *cur = head.next_;
    while (cur != nullptr) {
      head.next_ = cur->next_;
      delete cur;
      cur = head.next_;
    }
  }

  static bool Consume(Token **tok, char op) {
    if ((*tok)->kind_ != TK_RESERVED || (*tok)->str_[0] != op) {
      return false;
    }
    NextToken(tok);
    return true;
  }

  static void Expect(Token **tok, char op) {
    if ((*tok)->kind_ != TK_RESERVED || (*tok)->str_[0] != op) {
      ErrorAt(Token::prg_, (*tok)->str_, "expect '%c'", op);
    }
    NextToken(tok);
  }

  static long ExpectNumber(Token **tok) {
    if ((*tok)->kind_ != TK_NUM) {
      ErrorAt(prg_, (*tok)->str_, "expect a number");
    }
    long val = (*tok)->val_;
    NextToken(tok);
    return val;
  }

  static bool IsEof(Token *tok) { return tok->kind_ == TK_EOF; }

private:
  static void NextToken(Token **tok) { *tok = (*tok)->next_; }

  static char* prg_;

  Tokenkind kind_;
  long val_;
  char *str_;

  Token *next_;
};

char* Token::prg_ = nullptr;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
    return 1;
  }

  Token head;
  Token *cur = Token::Tokenize(head, argv[1]);

  std::cout << ".intel_syntax noprefix" << std::endl;
  std::cout << ".global main" << std::endl;
  std::cout << "main:" << std::endl;
  std::cout << "  mov rax," << Token::ExpectNumber(&cur) << std::endl;

  while (!Token::IsEof(cur)) {
    if (Token::Consume(&cur, '+')) {
      std::cout << "  add rax," << Token::ExpectNumber(&cur) << std::endl;
    }

    Token::Expect(&cur, '-');
    std::cout << "  sub rax," << head.ExpectNumber(&cur) << std::endl;
  }
  std::cout << "  ret" << std::endl;

  Token::TokenizeFree(head);

  return 0;
}
