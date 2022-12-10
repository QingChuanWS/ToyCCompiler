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
#ifndef VAR_GRUAD
#define VAR_GRUAD

#include "token.h"
#include "type.h"

class Var;

extern Var* locals;

class Var {
 public:
  Var(char* name = nullptr, Var* next = nullptr, Type* ty = nullptr,
      int offset = 0)
      : name_(name)
      , next_(next)
      , ty_(ty)
      , offset_(offset) {}

  Var*        Find(Token* tok);
  static void VarFree(Var* head);

  char* name_;     // variable name
  Type* ty_;       // Type;
  int   offset_;   // offset from rbp

  Var* next_;
};

#endif   // !VAR_GRUAD
