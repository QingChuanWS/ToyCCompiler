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

#ifndef TYPE_GRUAD
#define TYPE_GRUAD

#include <cstddef>
#include <memory>

#include "token.h"
#include "utils.h"

enum TypeKind {
  TY_VOID,
  TY_INT,
  TY_SHORT,
  TY_LONG,
  TY_CHAR,
  TY_PRT,
  TY_FUNC,
  TY_ARRAY,
  TY_STRUCT,
  TY_UNION,
  TY_END,
};

class Type {
 public:
  Type(TypeKind kind, int size, int align) : kind(kind), size(size), align(align) {}
  // copy construct.
  Type(const Type& ty) = default;
  // whether the type is integer.
  inline bool IsInteger() const;
  // whether the type is T.
  template <TypeKind T>
  inline bool Is() {
    return kind == T;
  }
  // whether the type is points.
  inline bool IsPointer() const { return base != nullptr; }
  // whether the type contains the tok name.
  inline bool HasName() const { return name != nullptr; }
  // get data size.
  inline int Size() const { return size; }
  // get base pointer size.
  inline int GetBaseSize() const { return base->size; }
  // get type's base type.
  const TypePtr& GetBase() const { return base; }
  // get type's name
  const TokenPtr& GetName() const;
  // get struct member based on token.
  MemberPtr GetStructMember(TokenPtr tok);
  // get type align
  inline int GetAlign() const { return align; }

 public:
  // create pointer type.
  static TypePtr CreatePointerType(TypePtr base);
  // create function type.
  static TypePtr CreateFunctionType(TypePtr ret_type, TypePtr params);
  // create array type.
  static TypePtr CreateArrayType(TypePtr base, int array_len);
  // create struct type.
  static TypePtr CreateStructType(MemberPtr mem);
  // create union type.
  static TypePtr CreateUnionType(MemberPtr mem);

 private:
  friend class Parser;
  friend class Object;

  // type kind
  TypeKind kind = TY_END;
  // Declaration.
  TokenPtr name = nullptr;
  // sizeof() value.
  int size = 0;
  // alignment
  int align = 0;
  // Pointer-to or array type. Using a same member to
  // represent pointer/array duality in C.
  TypePtr base = nullptr;
  // Array
  int array_len = 0;
  // Member
  MemberPtr mem = nullptr;
  // Function type.
  TypePtr return_ty = nullptr;
  // function params type list.
  TypePtr params = nullptr;
  // for params type list.
  TypePtr next = nullptr;
};

#endif  // !TYPE_GRUAD
