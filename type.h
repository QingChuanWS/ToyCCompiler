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
      , base_(nullptr)
      , return_ty_(nullptr)
      , params_(nullptr)
      , next_(nullptr) {}
  // copy construct.
  Type(const Type* ty);
  // create a pointer.
  Type(TypeKind kind, Type* ty);

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
  Type* return_ty_;
  Type* params_;
  // for params type list.
  Type* next_; 
};

#endif   // !TYPE_GRUAD
