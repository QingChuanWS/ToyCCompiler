/*
 * This project is exclusively owned by QingChuanWS and shall not be used for
 * commercial and profitting purpose without QingChuanWS's permission.
 *
 * @Author: bingshan45@163.com
 * Github: https://github.com/QingChuanWS
 * @Description: Token class declaration.
 *
 * Copyright (c) 2022 by QingChuanWS, All Rights Reserved.
 */
#ifndef TOKEN_GRUAD
#define TOKEN_GRUAD

#include <cctype>
#include <cstring>

#include "tools.h"

enum Tokenkind {
  TK_PUNCT,    // Punctuators,
  TK_IDENT,    // Identifiers,
  TK_KEYWORD,  // Keywords,
  TK_NUM,      // Number literals,
  TK_STR,      // String literals,
  TK_EOF,      // End-of-file markers,
};

class Type;
class Token {
 public:
  Token()
      : kind_(TK_EOF),
        next_(nullptr),
        val_(0),
        str_(nullptr),
        strlen_(0),
        ty_(nullptr),
        str_literal_(nullptr) {}

  Token(Tokenkind kind, char* str, int len)
      : kind_(kind),
        next_(nullptr),
        str_(str),
        strlen_(len),
        val_(0),
        ty_(nullptr),
        str_literal_(nullptr) {}

  Token(Tokenkind kind, char* start, char* end);

  Token(Tokenkind kind, Token* tok, char* str, int len)
      : kind_(kind), str_(str), strlen_(len), val_(0), next_(nullptr) {
    tok->next_ = this;
  }

  // Creating token list from the source program.
  static Token* TokenCreate(const Token& head, char* prg);
  // free token list.
  static void TokenFree(Token& head);
  // Check the current token->str is char op or not.
  // If the token's str is equal with op, return ture.
  Token* SkipToken(const char* op);
  // Check whether the current token's kind is EOF,
  // otherwise return false.
  bool IsEof() { return this->kind_ == TK_EOF; }
  // Check whether Token string equal special string
  bool Equal(const char* op);
  // Report an error in token list
  void ErrorTok(const char* fmt, ...);
  // Get tok name string based copy mode.
  char* GetIdent();
  // Get tok value when kind == NUM
  long GetNumber();
  // check whether the given token is a typename.
  bool IsTypename();

 private:
  // matching reserved keyword based start.
  void ConvertToReserved();
  // matching punction.
  int ReadPunct(char* p);
  // read a string literal for source pargram char.
  Token* ReadStringLiteral(char* p);
  // free token kind = TK_STR
  void StrTokenFree();

  friend class Node;
  friend class Object;

  // source code
  static char* prg_;
  // Token Kind
  Tokenkind kind_;
  // Next Token
  Token* next_;
  // If kind_ is TK_NUM, its values,
  long val_;
  // Token Location
  char* str_;
  // Token length
  int strlen_;
  // Use if kind_ is TK_STR
  Type* ty_;
  // String literal contents include terminating '\0'
  char* str_literal_;
};

#endif  //  TOKEN_GRUAD