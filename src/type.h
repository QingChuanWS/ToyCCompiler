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

#include <cstddef>
#include <memory>

#include "token.h"
#include "utils.h"

enum TypeKind {
  TY_INT,
  TY_CHAR,
  TY_PRT,
  TY_FUNC,
  TY_ARRAY,
  TY_STRUCT,
  TY_END,
};

class Type {
 public:
  Type(TypeKind kind, int size) : kind(kind), size(size) {}
  // copy construct.
  Type(const Type& ty) = default;
  // whether the type is integer.
  bool IsInteger() const;
  // whether the type is points.
  bool IsPointer() const;
  // whether the type is function.
  bool IsFunction() const;
  // whether the type is array
  bool IsArray() const;
  // whether the type is struct
  bool IsStruct() const;
  // whether the type contains the tok name.
  bool HasName();
  // get data size.
  int Size() { return size; }
  // get base pointer size.
  int GetBaseSize() { return base->size; }
  // get type's base type.
  const TypePtr& GetBase() const { return base; }
  // get type's name
  const TokenPtr& GetName() const;
  // get struct member based on token.
  StructPtr GetStructMember(TokenPtr tok);

 public:
  // create pointer type.
  static TypePtr CreatePointerType(TypePtr base);
  // create function type.
  static TypePtr CreateFunctionType(TypePtr ret_type, TypePtr params);
  // create array type.
  static TypePtr CreateArrayType(TypePtr base, int array_len);
  // create struct type.
  static TypePtr CreateStructType(StructPtr mem);

 private:
  friend class Parser;
  friend class Object;

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
  // Member
  StructPtr mem = nullptr;
  // Function type.
  TypePtr return_ty = nullptr;
  // function params type list.
  TypePtr params = nullptr;
  // for params type list.
  TypePtr next = nullptr;
};

#endif  // !TYPE_GRUAD
