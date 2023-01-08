/*
 * This project is exclusively owned by QingChuanWS and shall not be used for
 * commercial and profitting purpose without QingChuanWS's permission.
 *
 * @Author: bingshan45@163.com
 * Github: https://github.com/QingChuanWS
 * @Description:
 *
 * Copyright (c) 2023 by QingChuanWS, All Rights Reserved.
 */

#ifndef TOKEN_GRUAD
#define TOKEN_GRUAD

#include <cctype>
#include <cstring>

#include "tools.h"

class Object;

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
  explicit Token() = default;
  Token(Tokenkind kind, char* str, int len) : kind_(kind), str_(str), strlen_(len) {}
  // create string token.
  Token* CreateStringToken(char* start, char* end);
  // Creating token list from the source program.
  static Token* TokenCreate(const Token& head, char* prg);
  // free token list.
  static void TokenFree(Token& head);
  // Check the current token->str is char op or not.
  // If the token's str is equal with op, return ture.
  Token* SkipToken(const char* op, bool enable_error = true);
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
  // find a tok name whether is in locals variable list.
  Object* FindLocalVar();
  // find a tok name whether is in locals variable list.
  Object* FindGlobalVar();
  // consume a token if the given string is same the token string.
  Token* Consume(const char* op);

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
  Tokenkind kind_ = TK_EOF;
  // Next Token
  Token* next_ = nullptr;
  // If kind_ is TK_NUM, its values,
  long val_ = 0;
  // Token Location
  char* str_ = nullptr;
  // Token length
  int strlen_ = 0;
  // String literal contents include terminating '\0'
  char* str_literal_ = nullptr;
};

#endif  //  TOKEN_GRUAD