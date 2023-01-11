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

#include "utils.h"
#include "tools.h"

enum Tokenkind {
  TK_PUNCT,    // Punctuators,
  TK_IDENT,    // Identifiers,
  TK_KEYWORD,  // Keywords,
  TK_NUM,      // Number literals,
  TK_STR,      // String literals,
  TK_EOF,      // End-of-file markers,
};

class Token {
 public:
  Token(Tokenkind kind, char* str, int len) : kind(kind), str(str), strlen(len) {}
  // create string token.
  TokenPtr CreateStringToken(char* start, char* end);
  // Check the current token->str is char op or not.
  // If the token's str is equal with op, return ture.
  TokenPtr SkipToken(const char* op, bool enable_error = true);
  // Check whether the current token's kind is EOF,
  // otherwise return false.
  bool IsEof() { return this->kind == TK_EOF; }
  // Check whether Token string equal special string
  bool Equal(const char* op);
  // Report an error in token list
  void ErrorTok(const char* fmt, ...);
  // Get tok name string based copy mode.
  String GetIdent();
  // Get tok value when kind == NUM
  long GetNumber();
  // Check whether the given token is a typename.
  bool IsTypename();
  // Find a tok name whether is in locals variable list.
  ObjectPtr FindLocalVar();
  // Find a tok name whether is in locals variable list.
  ObjectPtr FindGlobalVar();

 public:
  // Creating token list from the source program.
  static TokenPtr TokenCreate(TokenPtr tok_list, char* prg);

 private:
  // find a closing double-quote.
  char* StringLiteralEnd(char* p);
  // read the escaped char
  int ReadEscapeedChar(char** new_pos, char* p);
  // convert char c to hex format
  int FromHex(char c);
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
  static char* prg;
  // Token Kind
  Tokenkind kind = TK_EOF;
  // Next Token
  TokenPtr next = nullptr;
  // If kind_ is TK_NUM, its values,
  long val = 0;
  // Token Location
  char* str = nullptr;
  // Token length
  int strlen = 0;
  // String literal contents include terminating '\0'
  String str_literal = String();
};

#endif  //  TOKEN_GRUAD