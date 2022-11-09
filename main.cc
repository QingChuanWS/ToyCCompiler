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
  exit(1);
}

class Token {
public:
  Token() : kind_(TK_EOF), next_(nullptr), val_(0), str_(nullptr) {}

  Token(Tokenkind kind, Token *cur, char *str) {
    kind_ = kind;
    str_ = str;
    cur->next_ = this;
  }

  friend class TokenList;

private:
  Tokenkind kind_;
  long val_;
  char *str_;

  Token *next_;
};

class TokenList {
public:
  TokenList(char *prg) : head_(), cur_(&head_) {
    while (*prg != '\0') {
      if (std::isdigit(*prg)) {
        cur_ = new Token(TK_NUM, cur_, prg);
        cur_->val_ = strtol(prg, &prg, 10);
        continue;
      }

      if (*prg == '+' || *prg == '-') {
        cur_ = new Token(TK_RESERVED, cur_, prg);
        prg++;
        continue;
      }

      if (std::isspace(*prg)) {
        prg++;
        continue;
      }

      Error("invalid token");
    }

    new Token(TK_EOF, cur_, prg);
    cur_ = head_.next_;
  }

  ~TokenList() {
    Token *cur_ = head_.next_;
    while (cur_ != nullptr) {
      head_.next_ = cur_->next_;
      delete cur_;
      cur_ = head_.next_;
    }
  }

  TokenList(const TokenList &) = delete;

  bool Consume(char op) {
    if (cur_->kind_ != TK_RESERVED || cur_->str_[0] != op) {
      return false;
    }
    NextToken();
    return true;
  }

  void Expect(char op) {
    if (cur_->kind_ != TK_RESERVED || cur_->str_[0] != op) {
      Error("expect '%c'", op);
    }
    NextToken();
  }

  long ExpectNumber() {
    if (cur_->kind_ != TK_NUM) {
      Error("expect a number");
    }
    long val = cur_->val_;
    NextToken();
    return val;
  }

  bool AtEof() { return cur_->kind_ == TK_EOF; }

  Token *GetCurrent() { return cur_; }

private:
  void NextToken() { cur_ = cur_->next_; }

  Token head_;
  Token *cur_;
};

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
    return 1;
  }

  TokenList tok_list(argv[1]);

  std::cout << ".intel_syntax noprefix" << std::endl;
  std::cout << ".global main" << std::endl;
  std::cout << "main:" << std::endl;
  std::cout << "  mov rax," << tok_list.ExpectNumber() << std::endl;

  while (!tok_list.AtEof()) {
    if (tok_list.Consume('+')) {
      std::cout << "  add rax," << tok_list.ExpectNumber() << std::endl;
    }

    tok_list.Expect('-');
    std::cout << "  sub rax," << tok_list.ExpectNumber() << std::endl;
  }
  std::cout << "  ret" << std::endl;

  return 0;
}
