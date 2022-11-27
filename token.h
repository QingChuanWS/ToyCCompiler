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
#ifndef TOKEN_GRUAD
#define TOKEN_GRUAD

#include "tools.h"

#include <cctype>
#include <cstring>

enum Tokenkind {
  TK_PUNCT,
  TK_IDENT,
  TK_KEYWORD,
  TK_NUM,
  TK_EOF,
};

class Token {
 public:
  Token()
      : kind_(TK_EOF)
      , next_(nullptr)
      , val_(0)
      , str_()
      , strlen_(0) {}

  Token(Tokenkind kind, char* str, int len)
      : kind_(kind)
      , str_(str)
      , strlen_(len)
      , val_(0)
      , next_(nullptr) {}

  Token(Tokenkind kind, Token* tok, char* str, int len)
      : kind_(kind)
      , str_(str)
      , strlen_(len)
      , val_(0)
      , next_(nullptr) {
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
  
 private:
  // matching reserved keyword based start.
  static void ConvertToReserved(Token* tok);
  // matching punction
  static int ReadPunct(char* p);

  friend class Function;
  friend class Node;
  friend class Var;
  static char* prg_;

  Tokenkind kind_;
  Token*    next_;
  long      val_;
  char*     str_;
  int       strlen_;
};

#endif   //  TOKEN_GRUAD