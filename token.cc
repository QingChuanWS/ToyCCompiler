/*
 * This project is exclusively owned by QingChuanWS and shall not be used for
 * commercial and profitting purpose without QingChuanWS's permission.
 *
 * @Author: bingshan45@163.com
 * Github: https://github.com/QingChuanWS
 * @Description: Token class defination.
 *
 * Copyright (c) 2022 by QingChuanWS, All Rights Reserved.
 */

#include "token.h"

#include <cctype>
#include <cstring>

#include "object.h"
#include "tools.h"
#include "type.h"

char* Token::prg_ = nullptr;

Token* Token::CreateStringToken(char* start, char* end){
  Token* res = new Token(TK_STR, start, end - start + 1);
  res->str_literal_ = strndup(start + 1, end - start - 1);
  return res;
}

Token* Token::TokenCreate(const Token& head, char* prg) {
  prg_ = prg;
  Token* cur = const_cast<Token*>(&head);
  char* p = prg_;
  while (*p != '\0') {
    if (std::isspace(*p)) {
      p++;
      continue;
    }

    if (std::isdigit(*p)) {
      cur->next_ = new Token(TK_NUM, p, 0);
      cur = cur->next_;
      char* q = p;
      cur->val_ = strtol(p, &p, 10);
      cur->strlen_ = static_cast<int>(p - q);
      continue;
    }

    if (*p == '"') {
      cur = cur->next_ = cur->ReadStringLiteral(p);
      p += cur->strlen_;
      continue;
    }

    // indentifier or keyword
    if (IsAlpha(*p)) {
      char* q = p++;
      while (IsAlnum(*p)) {
        p++;
      }
      cur = cur->next_ = new Token(TK_IDENT, q, p - q);
      continue;
    }

    int punct_len = cur->ReadPunct(p);
    if (punct_len) {
      cur = cur->next_ = new Token(TK_PUNCT, p, punct_len);
      p += punct_len;
      continue;
    }
    ErrorAt(prg, p, "expect a number.");
  }

  cur->next_ = new Token(TK_EOF, p, 0);
  head.next_->ConvertToReserved();
  return head.next_;
}

void Token::TokenFree(Token& head) {
  Token* cur = head.next_;
  while (cur != nullptr) {
    head.next_ = cur->next_;
    cur->StrTokenFree();
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

  return std::strchr("+-*/()<>;=,{}[]&", *p) != 0 ? 1 : 0;
}

void Token::ConvertToReserved() {
  static const char* kw[] = {"return", "if", "else", "for", "while", "int", "sizeof", "char"};
  for (Token* t = this; t != nullptr; t = t->next_) {
    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
      if (StrEqual(t->str_, kw[i], t->strlen_)) {
        t->kind_ = TK_KEYWORD;
      }
    }
  }
}

bool Token::Equal(const char* op) { return StrEqual(this->str_, op, this->strlen_); }

void Token::StrTokenFree() {
  if (this->kind_ == TK_STR) {
    free(this->str_literal_);
  }
}

Token* Token::SkipToken(const char* op, bool enable_error) {
  if (!this->Equal(op)) {
    if (enable_error) {
      ErrorAt(prg_, this->str_, "Expect \'%s\'", op);
    } else {
      return nullptr;
    }
  }
  return this->next_;
}

Token* Token::ReadStringLiteral(char* start) {
  char* p = start + 1;
  for (; *p != '"'; p++) {
    if (*p == '\n' || *p == '\0') {
      ErrorAt(Token::prg_, start, "unclosed string literal!");
    }
  }
  Token* tok = CreateStringToken(start, p);
  return tok;
}

void Token::ErrorTok(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = this->str_ - prg_;
  fprintf(stderr, "%s\n", prg_);
  fprintf(stderr, "%*s", pos, "");  // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

char* Token::GetIdent() {
  if (kind_ != TK_IDENT) {
    ErrorTok("GetIdent expect an identifier.");
  }
  return strndup(str_, strlen_);
}

long Token::GetNumber() {
  if (kind_ != TK_NUM) {
    ErrorTok("GetNumber expect an number.");
  }
  return val_;
}

Object* Token::FindLocalVar() { return locals->Find(this->str_); }

Object* Token::FindGlobalVar() { return globals->Find(this->str_); }

bool Token::IsTypename() { return Equal("int") || Equal("char"); }
