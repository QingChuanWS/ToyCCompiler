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
#include <memory>

#include "tools.h"

enum Tokenkind {
  TK_PUNCT,    // Punctuators,
  TK_IDENT,    // Identifiers,
  TK_KEYWORD,  // Keywords,
  TK_NUM,      // Number literals,
  TK_STR,      // String literals,
  TK_EOF,      // End-of-file markers,
};

class Token;
class Object;

using TokenPtr = std::shared_ptr<Token>;
using ObjectPtr = std::shared_ptr<Object>;

class Token {
 public:
  explicit Token() = default;
  Token(Tokenkind kind, char* str, int len) : kind_(kind), str_(str), strlen_(len) {}
  ~Token();
  // create string token.
  TokenPtr CreateStringToken(char* start, char* end);
  // Creating token list from the source program.
  static TokenPtr TokenCreate(TokenPtr tok_list, char* prg);
  // Check the current token->str is char op or not.
  // If the token's str is equal with op, return ture.
  TokenPtr SkipToken(const char* op, bool enable_error = true);
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
  // Check whether the given token is a typename.
  bool IsTypename();
  // Find a tok name whether is in locals variable list.
  ObjectPtr FindLocalVar();
  // Find a tok name whether is in locals variable list.
  ObjectPtr FindGlobalVar();

 private:
  // find a closing double-quote.
  static char* StringLiteralEnd(char* p);
  // matching reserved keyword based start.
  static void ConvertToReserved(TokenPtr tok);
  // matching punction.
  int ReadPunct(char* p);
  // read a string literal for source pargram char.
  TokenPtr ReadStringLiteral(char* p);
  // free token kind = TK_STR
  void StrTokenFree();

  friend class Node;
  friend class Object;

  // source code
  static char* prg_;
  // Token Kind
  Tokenkind kind_ = TK_EOF;
  // Next Token
  TokenPtr next_ = nullptr;
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