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

#include "token.h"

#include "tools.h"

#include <cctype>
#include <cstring>

char* Token::prg_ = nullptr;

Token* Token::TokenCreate(const Token& head, char* prg) {
  prg_       = prg;
  Token* cur = const_cast<Token*>(&head);
  char*  p   = prg_;
  while (*p != '\0') {
    if (std::isspace(*p)) {
      p++;
      continue;
    }

    if (std::isdigit(*p)) {
      cur->next_   = new Token(TK_NUM, p, 0);
      cur          = cur->next_;
      char* q      = p;
      cur->val_    = strtol(p, &p, 10);
      cur->strlen_ = static_cast<int>(p - q);
      continue;
    }

    // indentifier
    if (IsAlpha(*p)) {
      char* q = p++;
      while (IsAlnum(*p)) {
        p++;
      }
      cur->next_ = new Token(TK_IDENT, cur, q, p - q);
      cur        = cur->next_;
      continue;
    }

    int punct_len = ReadPunct(p);
    if (punct_len) {
      cur->next_ = new Token(TK_PUNCT, cur, p, punct_len);
      cur        = cur->next_;
      p += punct_len;
      continue;
    }

    ErrorAt(prg, p, "expect a number.");
  }

  cur->next_ = new Token(TK_EOF, p, 0);
  ConvertToReserved(head.next_);
  return head.next_;
}

void Token::TokenFree(Token& head) {
  Token* cur = head.next_;
  while (cur != nullptr) {
    head.next_ = cur->next_;
    delete cur;
    cur = head.next_;
  }
}

int Token::ReadPunct(char* p) {
  static const char* ops[] = {">=", "==", "!=", "<="};
  for (int i = 0; i < sizeof(ops) / sizeof(*ops); i++) {
    if (StrEqual(p, ops[i], 2)) {
      return 2;
    }
  }

  return std::strchr("+-*/()<>;={}&", *p) != 0 ? 1 : 0;
}

void Token::ConvertToReserved(Token* tok) {
  static const char* kw[] = {"return", "if", "else", "for", "while"};
  for (Token* t = tok; t != nullptr; t = t->next_) {
    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
      if (StrEqual(t->str_, kw[i], t->strlen_)) {
        t->kind_ = TK_IDENT;
      }
    }
  }
}

bool Token::Equal(const char* op) {
  return StrEqual(this->str_, op, this->strlen_);
}

Token* Token::SkipToken(const char* op) {
  if (!this->Equal(op)) {
    ErrorTok(prg_, this, "Expect '%s'", op);
  }
  return this->next_;
}

void Token::ErrorTok(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = this->str_ - prg_;
  fprintf(stderr, "%s\n", prg_);
  fprintf(stderr, "%*s", pos, "");   // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}