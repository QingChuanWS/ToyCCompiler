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
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>

#include "object.h"
#include "tools.h"
#include "utils.h"

String current_filename;

StringPtr prg;

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
  res->str_literal = std::move(new_str);
  return res;
}

StringStream Token::ReadFromStdin() {
  StringStream buf;

  buf << std::cin.rdbuf();
  return buf;
}

StringStream Token::ReadFromFile(const String& filename) {
  StringStream buf;

  std::ifstream input(filename);
  if (!input.is_open()) {
    Error("cannot open %s: %s", filename.c_str(), strerror(errno));
  }
  buf << input.rdbuf();
  input.close();
  return buf;
}

StringPtr Token::ReadFile(const String& filename) {
  StringStream buf;
  // By convention, read from the stdin if the given file name is '-'.
  if (filename == "-") {
    buf = ReadFromStdin();
  } else {
    buf = ReadFromFile(filename);
  }

  String program(buf.str());
  // Make sure that the last line is properly terminated with '\n'
  if (program.size() == 0 || program[program.size() - 1] != '\n') {
    program.push_back('\n');
  }
  program.push_back('\0');

  return std::make_shared<String>(std::move(program));
}

TokenPtr Token::CreateTokens(const String& file_name, const StringPtr& program) {
  current_filename = file_name;
  prg = program;
  TokenPtr tok_list = std::make_shared<Token>(TK_EOF, nullptr, 0);
  TokenPtr cur = tok_list;
  char* p = &(*prg)[0];
  while (*p != '\0') {
    if (std::isspace(*p)) {
      p++;
      continue;
    }

    if (std::isdigit(*p)) {
      cur = cur->next = std::make_shared<Token>(TK_NUM, p, 0);
      char* q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = static_cast<int>(p - q);
      continue;
    }

    if (*p == '"') {
      cur = cur->next = cur->ReadStringLiteral(p);
      p += cur->len;
      continue;
    }

    // indentifier or keyword
    if (IsAlpha(*p)) {
      char* q = p++;
      while (IsAlnum(*p)) {
        p++;
      }
      cur = cur->next = std::make_shared<Token>(TK_IDENT, q, p - q);
      continue;
    }

    int punct_len = cur->ReadPunct(p);
    if (punct_len) {
      cur = cur->next = std::make_shared<Token>(TK_PUNCT, p, punct_len);
      p += punct_len;
      continue;
    }
    ErrorAt(p, "expect a number.");
  }

  cur->next = std::make_shared<Token>(TK_EOF, p, 0);
  ConvertToReserved(tok_list->next);
  return tok_list->next;
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
  for (TokenPtr t = tok; t != nullptr; t = t->next) {
    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
      if (StrEqual(t->loc, kw[i], t->len)) {
        t->kind = TK_KEYWORD;
      }
    }
  }
}

bool Token::Equal(const char* op) { return StrEqual(this->loc, op, this->len); }

TokenPtr Token::SkipToken(const char* op, bool enable_error) {
  if (!this->Equal(op)) {
    if (enable_error) {
      ErrorAt(this->loc, "Expect \'%s\'", op);
    } else {
      return nullptr;
    }
  }
  return this->next;
}

int Token::FromHex(char c) {
  if ('0' <= c && c <= '9') {
    return c - '0';
  }
  if ('a' <= c && c <= 'f') {
    return c - 'a' + 10;
  }
  return c - 'A' + 10;
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

  if (*p == 'x') {
    // return a hexadecimal number.
    p++;
    if (!std::isxdigit(*p)) {
      ErrorAt(p, "invaild hex escape sequence.");
    }
    int c = 0;
    for (; isxdigit(*p); p++) {
      c = (c << 4) + FromHex(*p);
    }
    *new_pos = p;
    return c;
  }

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
      ErrorAt(start, "unclosed string literal!");
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

String Token::GetIdent() {
  if (kind != TK_IDENT) {
    ErrorTok("GetIdent expect an identifier.");
  }
  return String(loc, len);
}

long Token::GetNumber() {
  if (kind != TK_NUM) {
    ErrorTok("GetNumber expect an number.");
  }
  return val;
}

ObjectPtr Token::FindLocalVar() { return Object::Find(locals, this->loc); }

ObjectPtr Token::FindGlobalVar() { return Object::Find(globals, this->loc); }

bool Token::IsTypename() { return Equal("int") || Equal("char"); }

TokenPtr Token::TokenizeFile(const String& file_name) {
  return CreateTokens(file_name, ReadFile(file_name));
}

void Token::ErrorTok(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  VrdicErrorAt(this->loc, fmt, ap);
}

void Token::ErrorAt(char* loc, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  VrdicErrorAt(loc, fmt, ap);
}

void Token::VrdicErrorAt(char* loc, const char* fmt, va_list ap) {
  // find a line containing `loc`
  char* start = loc;
  const char* current_input = prg->c_str();
  while (current_input < start && start[-1] != '\n') {
    start--;
  }
  // get the current line end.
  char* end = loc;
  while (*end != '\n') {
    end++;
  }
  // get line number
  int line_no = 1;
  for (const char* p = current_input; p < start; p++) {
    if (*p == '\n') {
      line_no++;
    }
  }
  // print the line
  int indent = fprintf(stderr, "%s:%d: ", current_filename.c_str(), line_no);
  fprintf(stderr, "%.*s\n", static_cast<int>(end - start), start);
  int pos = static_cast<int>(loc - start + indent);

  fprintf(stderr, "%*s", pos, "");  // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}