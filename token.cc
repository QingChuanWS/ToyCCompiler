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

    // keyword and ops match.
    const char* kw = StartWithReserved(p);
    if (kw != nullptr) {
      int kw_size = std::strlen(kw);
      cur->next_  = new Token(TK_RESERVED, p, kw_size);
      cur         = cur->next_;
      p += kw_size;
      continue;
    }

    if (std::strchr("+-*/()<>;=", *p)) {
      cur->next_ = new Token(TK_RESERVED, p, 1);
      cur        = cur->next_;
      p++;
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

    if (std::isdigit(*p)) {
      cur->next_   = new Token(TK_NUM, p, 0);
      cur          = cur->next_;
      char* q      = p;
      cur->val_    = strtol(p, &p, 10);
      cur->strlen_ = static_cast<int>(q - p);
      continue;
    }

    ErrorAt(prg, p, "expect a number.");
  }

  cur->next_ = new Token(TK_EOF, p, 0);
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

const char* Token::StartWithReserved(char* p) {
  static const char* kw[] = {"return", "if", "else"};
  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
    int kw_size = std::strlen(kw[i]);
    if (StartSwith(p, kw[i], kw_size) && !IsAlnum(p[kw_size])) {
      return kw[i];
    }
  }

  static const char* ops[] = {">=", "==", "!=", "<="};
  for (int i = 0; i < sizeof(ops) / sizeof(*ops); i++) {
    int op_size = std::strlen(ops[i]);
    if (StartSwith(p, ops[i], op_size)) {
      return ops[i];
    }
  }

  return nullptr;
}

bool Token::Consume(Token** tok, const char* op) {
  Token* cur_tok = *tok;
  if (cur_tok->kind_ != TK_RESERVED || std::strlen(op) != cur_tok->strlen_ ||
      !StartSwith(cur_tok->str_, op, cur_tok->strlen_)) {
    return false;
  }
  NextToken(tok);
  return true;
}

Token* Token::ConsumeIdent(Token** tok) {
  Token* cur_tok = *tok;
  if (cur_tok->kind_ != TK_IDENT) {
    return nullptr;
  }
  NextToken(tok);
  return cur_tok;
}

void Token::Expect(Token** tok, const char* op) {
  Token* cur_tok = *tok;
  if (cur_tok->kind_ != TK_RESERVED || std::strlen(op) != cur_tok->strlen_ ||
      !StartSwith(cur_tok->str_, op, cur_tok->strlen_)) {
    ErrorAt(Token::prg_, (*tok)->str_, "expect \"%s\"", op);
  }
  NextToken(tok);
}

long Token::ExpectNumber(Token** tok) {
  Token* cur_tok = *tok;
  if (cur_tok->kind_ != TK_NUM) {
    ErrorAt(prg_, cur_tok->str_, "expect a number");
  }
  long val = cur_tok->val_;
  NextToken(tok);
  return val;
}
