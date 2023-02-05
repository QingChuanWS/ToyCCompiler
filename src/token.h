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

#ifndef TOKEN_GRUAD
#define TOKEN_GRUAD

#include <cctype>
#include <cstring>
#include <memory>

#include "tools.h"
#include "utils.h"

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
  Token(Tokenkind kind, const char* str, const int len) : kind(kind), loc(str), len(len) {}
  // whether the tok kind is the T.
  template <Tokenkind T>
  bool Is() {
    return kind == T;
  }
  // create string token.
  TokenPtr CreateStringToken(const char* start, const char* end);
  // Check the current token->str is char op or not.
  // If the token's str is equal with op, return ture.
  TokenPtr SkipToken(const char* op, bool enable_error = true);
  // Check whether the current token's kind is EOF,
  // otherwise return false.
  inline bool IsEof() { return kind == TK_EOF; }
  // Check whether Token string equal special string
  bool Equal(const char* op) const;
  // Check whether Token string equal special tok
  bool Equal(const TokenPtr tok) const;
  // Report an error in token list
  void ErrorTok(const char* fmt, ...) const;
  // Get tok name string based copy mode.
  String GetIdent() const;
  // Get tok value when kind == NUM
  long GetNumber() const;
  // Get tok line number.
  int GetLineNo() const;
  // Check whether the given token is a typename.
  bool IsTypename() const;
  // get the tok next point
  inline const TokenPtr& GetNext() const { return next; }
  // get the tok kind.
  inline Tokenkind GetKind() const { return kind; }

 private:
  // find a closing double-quote.
  const char* StringLiteralEnd(const char* start);
  // read the escaped char
  static int ReadEscapeedChar(const char** new_pos, const char* p);
  // convert char c to hex format
  static int FromHex(const char c);
  // matching punction.
  int ReadPunct(const char* p) const;
  // read a string literal for source pargram char.
  TokenPtr ReadStringLiteral(const char* start);
  // read character literal
  static TokenPtr ReadCharacterLiteral(const char* start);

 public:
  // Create
  static TokenPtr TokenizeFile(const String& file_name);

 private:
  // read code from filename.
  static StringStream ReadFromFile(const String& filename);
  // return the contents of given file.
  static StringPtr ReadFile(const String& filename);
  // creating token list from the source program.
  static TokenPtr CreateTokens(const String& file_name, const StringPtr& program);
  // matching reserved keyword based start.
  static void ConvertToReserved(TokenPtr tok);
  // initializa the line info of all token.
  static void InitLineNumInfo(TokenPtr tok);
  // Reports an error location and exit.
  static void ErrorAt(const char* loc, const char* fmt, ...);
  // Reports an error message in the follow format.
  //
  // foo.c:10: x = y + 1;
  //               ^ <error message here>
  static void VrdicErrorAt(int line_on, const char* loc, const char* fmt, va_list ap);

  friend class Parser;
  friend class Type;

 private:
  // Token Kind
  Tokenkind kind = TK_EOF;
  // Next Token
  TokenPtr next = nullptr;
  // If kind_ is TK_NUM, its values,
  int64_t val = 0;
  // Token Location
  const char* loc;
  // Token length
  int len = 0;
  // String literal contents include terminating '\0'
  String str_literal = "";
  // token line number
  int line_no = -1;
};

#endif  //  TOKEN_GRUAD