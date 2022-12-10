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
#ifndef TYPE_GRUAD
#define TYPE_GRUAD

#include "token.h"

class Type;

extern Type* ty_int;

enum TypeKind {
  TY_INT,
  TY_PRT,
  TY_END,
};

class Type {
 public:
  explicit Type()
      : kind_(TY_END)
      , base_(nullptr) {}
  // create a pointer
  Type(TypeKind kind, Type* base)
      : kind_(kind)
      , base_(base) {}

  bool IsInteger();
  bool IsPointer();

  static void TypeFree(Type* ty);

 private:
  friend class Node;
  TypeKind kind_;

  // pointer
  Type* base_;

  // Declaration
  Token* name;
};

#endif   // !TYPE_GRUAD
