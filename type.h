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

class Type;

extern std::shared_ptr<Type> ty_int;
extern std::shared_ptr<Type> ty_char;

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
  explicit Type() = default;
  explicit Type(TypeKind kind, int size) : kind_(kind), size_(size) {}
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
  static Type* CreatePointerType(Type* base);
  // create function type.
  static Type* CreateFunctionType(Type* ret_type, Type* params);
  // create array type
  static Type* CreateArrayType(Type* base, int array_len);
  // free type list.
  static void TypeFree(Type* ty);

 private:
  friend class Node;
  friend class Object;
  friend class CodeGenerator;
  // type kind
  TypeKind kind_ = TY_END;
  // Declaration.
  Token* name_ = nullptr;
  // sizeof() value.
  int size_ = 0;
  // Pointer-to or array type. Using a same member to
  // represent pointer/array duality in C.
  Type* base_ = nullptr;
  // Array
  int array_len_ = 0;
  // Function type.
  Type* return_ty_ = nullptr;
  // function params type list.
  Type* params_ = nullptr;
  // for params type list.
  Type* next_ = nullptr;
};

#endif  // !TYPE_GRUAD
