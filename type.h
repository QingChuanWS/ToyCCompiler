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
  TY_FUNC,
  TY_END,
};

class Type {
 public:
  Type()
      : kind_(TY_END)
      , name_(nullptr)
      , base_(nullptr) {}
  // create a pointer
  Type(TypeKind kind, Type* ty)
      : kind_(kind)
      , name_(nullptr) {
    if (kind_ == TY_PRT) {
      base_ = ty;
      return_ty = nullptr;
    }
    if (kind_ == TY_FUNC) {
      return_ty = ty;
      base_ = nullptr;
    }
  }

  bool IsInteger();
  bool IsPointer();

  static void TypeFree(Type* ty);

 private:
  friend class Node;
  friend class Function;
  
  TypeKind kind_;
  // Declaration
  Token* name_;

  // pointer
  Type* base_;
  // Function type
  Type* return_ty;
};

#endif   // !TYPE_GRUAD
