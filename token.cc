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
#include <memory>

#include "object.h"
#include "tools.h"

char* Token::prg_ = nullptr;

using TokenPtr = std::shared_ptr<Token>;

TokenPtr Token::CreateStringToken(char* start, char* end) {
  int max_len = static_cast<int>(end - start);
  String new_str = String(max_len, '\0');
  int len = 0;
  for (char* p = start + 1; p < end;) {
    if (*p == '\\') {
      new_str[len++] = ReadEscapeedChar(&p, p + 1);
    } else {
      new_str[len++] = *p++;
    }
  }
  TokenPtr res = std::make_shared<Token>(TK_STR, start, end - start + 1);
  res->str_literal_ = std::move(new_str);
  return res;
}

TokenPtr Token::TokenCreate(TokenPtr tok_list, char* prg) {
  prg_ = prg;
  TokenPtr cur = tok_list;
  char* p = prg_;
  while (*p != '\0') {
    if (std::isspace(*p)) {
      p++;
      continue;
    }

    if (std::isdigit(*p)) {
      cur = cur->next_ = std::make_shared<Token>(TK_NUM, p, 0);
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
      cur = cur->next_ = std::make_shared<Token>(TK_IDENT, q, p - q);
      continue;
    }

    int punct_len = cur->ReadPunct(p);
    if (punct_len) {
      cur = cur->next_ = std::make_shared<Token>(TK_PUNCT, p, punct_len);
      p += punct_len;
      continue;
    }
    ErrorAt(prg, p, "expect a number.");
  }

  cur->next_ = std::make_shared<Token>(TK_EOF, p, 0);
  ConvertToReserved(tok_list->next_);
  return tok_list->next_;
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

void Token::ConvertToReserved(TokenPtr tok) {
  static const char* kw[] = {"return", "if", "else", "for", "while", "int", "sizeof", "char"};
  for (TokenPtr t = tok; t != nullptr; t = t->next_) {
    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
      if (StrEqual(t->str_, kw[i], t->strlen_)) {
        t->kind_ = TK_KEYWORD;
      }
    }
  }
}

bool Token::Equal(const char* op) { return StrEqual(this->str_, op, this->strlen_); }

TokenPtr Token::SkipToken(const char* op, bool enable_error) {
  if (!this->Equal(op)) {
    if (enable_error) {
      ErrorAt(prg_, this->str_, "Expect \'%s\'", op);
    } else {
      return nullptr;
    }
  }
  return this->next_;
}

int Token::ReadEscapeedChar(char** new_pos, char* p) {
  if ('0' <= *p && *p <= '7') {
    //  Read an octal number.
    int c = static_cast<int>(*p++ - '0');
    if ('0' <= *p && *p <= '7') {
      c = (c << 3) + (*p++ - '0');
      if ('0' <= *p && *p <= '7') {
        c = (c << 3) + (*p++ - '0');
      }
    }
    *new_pos = p;
    return c;
  }
  *new_pos = p + 1;
  // Escape sequences are defined using themselves here. E.g.
  // '\n' is implemented using '\n'. This tautological definition
  // works because the compiler that compiles our compiler knows
  // what '\n' actually is. In other words, we "inherit" the ASCII
  // code of '\n' from the compiler that compiles our compiler,
  // so we don't have to teach the actual code here.
  //
  // This fact has huge implications not only for the correctness
  // of the compiler but also for the security of the generated code.
  switch (*p) {
    case 'a':
      return '\a';
    case 'b':
      return '\b';
    case 't':
      return '\t';
    case 'n':
      return '\n';
    case 'v':
      return '\v';
    case 'f':
      return '\f';
    case 'r':
      return '\r';
    // [GNU] \e is a escaped char in gnu extension.
    case 'e':
      return 27;
    default:
      return *p;
  }
}

char* Token::StringLiteralEnd(char* start) {
  char* p = start + 1;
  for (; *p != '"'; p++) {
    if (*p == '\n' || *p == '\0') {
      ErrorAt(Token::prg_, start, "unclosed string literal!");
    }
    if (*p == '\\') {
      p++;
    }
  }
  return p;
}

TokenPtr Token::ReadStringLiteral(char* start) {
  char* end = StringLiteralEnd(start);
  TokenPtr tok = CreateStringToken(start, end);
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

ObjectPtr Token::FindLocalVar() { return Object::Find(locals, this->str_); }

ObjectPtr Token::FindGlobalVar() { return Object::Find(globals, this->str_); }

bool Token::IsTypename() { return Equal("int") || Equal("char"); }
