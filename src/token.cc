/*
 * This project is exclusively owned by QingChuanWS and shall not be used for
 * commercial and profitting purpose without QingChuanWS's permission.
 *
 * @ Author: bingshan45@163.com
 * @ Github: https://github.com/QingChuanWS
 * @ Description:
 *
 * Copyright (c) 2023 by QingChuanWS, All Rights Reserved.
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
#include <vector>

#include "object.h"
#include "scope.h"
#include "tools.h"
#include "utils.h"

// intput file name.
String current_filename;

// input string.
StringPtr prg;

TokenPtr Token::CreateStringToken(const char* start, const char* end) const {
  auto max_len = static_cast<int>(end - start);
  auto new_str = String(max_len, '\0');
  int str_litral_len = 0;
  for (const char* p = start + 1; p < end;) {
    if (*p == '\\') {
      new_str[str_litral_len++] = ReadEscapeedChar(&p, p + 1);
    } else {
      new_str[str_litral_len++] = *p++;
    }
  }
  auto res = std::make_shared<Token>(TK_STR, start, end - start + 1);
  res->str_literal = std::move(new_str);
  return res;
}

StringStream Token::ReadFromFile(const String& filename) {
  StringStream buf;
  // By convention, read from the stdin if the given file name is '-'.
  if (filename == "-") {
    buf << std::cin.rdbuf();
    return buf;
  }

  std::ifstream input(filename);
  if (!input.is_open()) {
    Error("cannot open %s: %s", filename.c_str(), strerror(errno));
  }
  buf << input.rdbuf();
  input.close();
  return buf;
}

StringPtr Token::ReadFile(const String& filename) {
  auto program = String(ReadFromFile(filename).str());
  // Make sure that the last line is properly terminated with '\n'
  if (program.size() == 0 || program[program.size() - 1] != '\n') {
    program.push_back('\n');
  }
  program.push_back('\0');

  return std::make_shared<String>(std::move(program));
}

TokenPtr Token::ReadCharacterLiteral(const char* start) {
  const char* p = start + 1;
  if (*p == '\0') {
    ErrorAt(start, "uncloser character literal.");
  }
  char c;
  if (*p == '\\') {
    c = ReadEscapeedChar(&p, p + 1);
  } else {
    c = *p++;
  }

  const char* end = std::strchr(p, '\'');
  if (!end) {
    ErrorAt(p, "uncloser character literal.");
  }
  auto res = std::make_shared<Token>(TK_NUM, start, end - start + 1);
  res->val = c;
  return res;
}

TokenPtr Token::CreateTokens(const String& file_name, const StringPtr& program) {
  current_filename = file_name;
  prg = program;
  auto tok_list = std::make_shared<Token>(TK_EOF, nullptr, 0);
  TokenPtr cur = tok_list;
  char* p = &(*prg)[0];
  while (*p != '\0') {
    // skip line comments.
    if (StrEqual(p, "//", 2)) {
      p += 2;
      while (*p != '\n') {
        p++;
      }
      continue;
    }

    // skip block comments.
    if (StrEqual(p, "/*", 2)) {
      char* q = strstr(p + 2, "*/");
      if (!q) {
        ErrorAt(p, "unclose block comment.");
      }
      p = q + 2;
      continue;
    }

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

    if (*p == '\'') {
      cur = cur->next = ReadCharacterLiteral(p);
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
  InitLineNumInfo(tok_list->next);
  return tok_list->next;
}

int Token::ReadPunct(const char* p) const {
  static std::vector<const char*> ops = {">=", "==", "!=", "<=", "->"};
  for (auto& op : ops) {
    if (StrEqual(p, op, 2)) {
      return 2;
    }
  }

  return std::strchr("+-*/()<>;=,{}[]&.", *p) != nullptr ? 1 : 0;
}

void Token::ConvertToReserved(TokenPtr tok) {
  static std::vector<const char*> keyword = {
      "return", "if",    "else", "for",  "while",   "int",   "sizeof", "char",  "struct",
      "union",  "short", "long", "void", "typedef", "_Bool", "enum",   "static"};
  for (TokenPtr t = tok; t != nullptr; t = t->next) {
    for (auto& kw : keyword) {
      if (StrEqual(t->loc, kw, t->len)) {
        t->kind = TK_KEYWORD;
      }
    }
  }
}

void Token::InitLineNumInfo(TokenPtr tok) {
  const char* p = prg->c_str();
  int n = 1;

  do {
    if (p == tok->loc) {
      tok->line_no = n;
      tok = tok->next;
    }

    if (*p == '\n') {
      n++;
    }
  } while (*p++);
}

bool Token::Equal(const char* op) const { return StrEqual(loc, op, len); }

bool Token::Equal(const TokenPtr tok) const {
  return tok->len == len && !std::strncmp(loc, tok->loc, len);
}

const TokenPtr& Token::SkipToken(const char* op, bool enable_error) const {
  if (!Equal(op) && enable_error) {
    ErrorAt(this->loc, "Expect \'%s\'", op);
  }
  return next;
}

int Token::FromHex(const char c) {
  if ('0' <= c && c <= '9') {
    return c - '0';
  }
  if ('a' <= c && c <= 'f') {
    return c - 'a' + 10;
  }
  return c - 'A' + 10;
}

int Token::ReadEscapeedChar(const char** new_pos, const char* p) {
  if ('0' <= *p && *p <= '7') {
    //  Read an octal number.
    auto c = static_cast<char>(*p++ - '0');
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

const char* Token::StringLiteralEnd(const char* start) const {
  const char* p = start + 1;
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

TokenPtr Token::ReadStringLiteral(const char* start) const {
  const char* end = StringLiteralEnd(start);
  TokenPtr tok = CreateStringToken(start, end);
  return tok;
}

String Token::GetIdent() const {
  if (kind != TK_IDENT) {
    ErrorTok("GetIdent expect an identifier.");
  }
  return String(loc, len);
}

long Token::GetNumber() const {
  if (kind != TK_NUM) {
    ErrorTok("GetNumber expect an number.");
  }
  return val;
}

int Token::GetLineNo() const { return line_no; }

bool Token::IsTypename() const {
  static std::vector<const char*> keyword = {"void",    "char",   "short", "int",
                                             "long",    "struct", "union", "struct",
                                             "typedef", "_Bool",  "enum",  "static"};
  for (auto& kw : keyword) {
    if (Equal(kw)) {
      return true;
    }
  }
  return Scope::FindTypedef(std::make_shared<Token>(*this)) != nullptr;
}

TokenPtr Token::TokenizeFile(const String& input_file) {
  return CreateTokens(input_file, ReadFile(input_file));
}

void Token::ErrorTok(const char* fmt, ...) const {
  va_list ap;
  va_start(ap, fmt);
  VrdicErrorAt(this->line_no, this->loc, fmt, ap);
  exit(1);
}

void Token::ErrorAt(const char* loc, const char* fmt, ...) {
  // get line number
  int line_no = 1;
  for (const char* p = prg->c_str(); p < loc; p++) {
    if (*p == '\n') {
      line_no++;
    }
  }
  va_list ap;
  va_start(ap, fmt);
  VrdicErrorAt(line_no, loc, fmt, ap);
  exit(1);
}

void Token::VrdicErrorAt(int line_no, const char* loc, const char* fmt, va_list ap) {
  // find a line containing `loc`
  const char* start = loc;
  const char* current_input = prg->c_str();
  while (current_input < start && start[-1] != '\n') {
    start--;
  }
  // get the current line end.
  const char* end = loc;
  while (*end != '\n') {
    end++;
  }
  // print the line
  const int indent = fprintf(stderr, "%s:%d: ", current_filename.c_str(), line_no);
  fprintf(stderr, "%.*s\n", static_cast<int>(end - start), start);
  auto pos = static_cast<int>(loc - start + indent);

  fprintf(stderr, "%*s", pos, "");  // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}

template <>
const TokenPtr& Token::GetNext<1>(TokenPtr& tok) {
  return tok->next;
};