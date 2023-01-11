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
#ifndef TYPE_GRUAD
#define TYPE_GRUAD

#include <memory>

#include "token.h"
#include "utils.h"

enum TypeKind {
  TY_INT,
  TY_CHAR,
  TY_PRT,
  TY_FUNC,
  TY_ARRAY,
  TY_END,
};

class Type {
 public:
  Type(TypeKind kind, int size) : kind(kind), size(size) {}
  // copy construct.
  explicit Type(const Type& ty) = default;
  // whether the type is integer.
  bool IsInteger();
  // whether the type is points.
  bool IsPointer();
  // whether the type is function.
  bool IsFunction();
  // whether the type contains the tok name.
  bool HasName();

 public:
  // create pointer type.
  static TypePtr CreatePointerType(TypePtr base);
  // create function type.
  static TypePtr CreateFunctionType(TypePtr ret_type, TypePtr params);
  // create array type
  static TypePtr CreateArrayType(TypePtr base, int array_len);

 private:
  friend class Node;
  friend class Object;
  friend class CodeGenerator;
  // type kind
  TypeKind kind = TY_END;
  // Declaration.
  TokenPtr name = nullptr;
  // sizeof() value.
  int size = 0;
  // Pointer-to or array type. Using a same member to
  // represent pointer/array duality in C.
  TypePtr base = nullptr;
  // Array
  int array_len = 0;
  // Function type.
  TypePtr return_ty = nullptr;
  // function params type list.
  TypePtr params = nullptr;
  // for params type list.
  TypePtr next = nullptr;
};

#endif  // !TYPE_GRUAD
