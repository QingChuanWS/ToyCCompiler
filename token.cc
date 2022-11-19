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

char* Token::prg_ = nullptr;

Token* Token::TokenCreate(Token& head, char* prg) {
  prg_       = prg;
  Token* cur = &head;
  char*  p   = prg_;
  while (*p != '\0') {
    if (std::isdigit(*p)) {
      cur->next_   = new Token(TK_NUM, p, 0);
      cur          = cur->next_;
      char* q      = p;
      cur->val_    = strtol(p, &p, 10);
      cur->strlen_ = static_cast<int>(q - p);
      continue;
    }

    if (std::ispunct(*p)) {
      cur->next_ = new Token(TK_RESERVED, p, 1);
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

bool Token::Consume(Token** tok, const char* op) {
  if ((*tok)->kind_ != TK_RESERVED || strlen(op) != (*tok)->strlen_ ||
      strncmp((*tok)->str_, op, (*tok)->strlen_) != 0) {
    return false;
  }
  NextToken(tok);
  return true;
}

void Token::Expect(Token** tok, const char* op) {
  if ((*tok)->kind_ != TK_RESERVED || strlen(op) != (*tok)->strlen_ ||
      strncmp((*tok)->str_, op, (*tok)->strlen_) != 0) {
    ErrorAt(Token::prg_, (*tok)->str_, "expect \"%s\"", op);
  }
  NextToken(tok);
}

long Token::ExpectNumber(Token** tok) {
  if ((*tok)->kind_ != TK_NUM) {
    ErrorAt(prg_, (*tok)->str_, "expect a number");
  }
  long val = (*tok)->val_;
  NextToken(tok);
  return val;
}
