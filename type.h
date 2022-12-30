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
  TY_ARRAY,
  TY_END,
};

class Type {
 public:
  Type()
      : kind_(TY_END)
      , name_(nullptr)
      , size_(0)
      , base_(nullptr)
      , array_len_(0)
      , return_ty_(nullptr)
      , params_(nullptr)
      , next_(nullptr) {}
  // copy construct.
  Type(const Type* ty);
  // create a pointer.
  Type(TypeKind kind, Type* base, int len = 0);

  bool IsInteger();
  bool IsPointer();

  static void TypeFree(Type* ty);

 private:
  friend class Node;
  friend class Object;
  friend class CodeGenerator;

  TypeKind kind_;
  // Declaration.
  Token* name_;
  // sizeof() value.
  int size_;
  // Pointer-to or array type. Using a same member to
  // represent pointer/array duality in C.
  Type* base_;
  // Array
  int array_len_;
  // Function type
  Type* return_ty_;
  Type* params_;
  Type* next_;   // for params type list.
};

#endif   // !TYPE_GRUAD
