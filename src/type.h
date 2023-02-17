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

#include <memory>

#include "struct.h"
#include "token.h"
#include "utils.h"

enum TypeKind {
  TY_VOID,
  TY_BOOL,
  TY_CHAR,
  TY_INT,
  TY_SHORT,
  TY_LONG,
  TY_ENUM,
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
  // copy type list.
  // whether the type is integer.
  bool IsInteger() const;
  // whether the type is T.
  template <TypeKind T>
  inline bool Is() const {
    return kind == T;
  }
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
  MemberPtr GetStructMember(TokenPtr tok) const;
  // get type align
  inline int GetAlign() const { return align; }

 public:
  // create pointer type.
  static TypePtr CreatePointerType(TypePtr base);
  // create function type.
  static TypePtr CreateFunctionType(TypePtr ret_type, const TypePtrVector& params);
  // create array type.
  static TypePtr CreateArrayType(TypePtr base, int array_len);
  // create struct type.
  static TypePtr CreateStructType(MemPtrVector mem, TokenPtr tag);
  // create union type.
  static TypePtr CreateUnionType(MemPtrVector mem, TokenPtr tag);
  // create enum type.
  static TypePtr CreateEnumType();
  // Inference Node Type
  static void TypeInfer(NodePtr node);
  // Is same Struct
  void UpdateStructMember(const MemPtrVector& mem);

 private:
  friend class Parser;
  friend class Object;
  friend class Node;

  // helper func: get ty1, ty2 common type.
  static TypePtr GetCommonType(const TypePtr& ty1, const TypePtr& ty2);
  // usual arithmetic convert
  static void UsualArithConvert(NodePtr& lhs, NodePtr& rhs);
  // whether two struct has same tag in same scope.
  bool IsSameStruct(TypePtr ty1);

  // type kind
  TypeKind kind = TY_END;
  // Declaration.
  TokenPtr name = nullptr;
  // sizeof() value.
  int size = 0;
  // alignment
  int align = 0;

  // ---- pointer ----
  // Pointer-to or array type. Using a same member to
  // represent pointer/array duality in C.
  TypePtr base = nullptr;
  TypeWeakPtr base_weak;
  bool is_self_pointer = false;

  // ---- Array ----
  int array_len = 0;

  // ---- Member---
  TokenPtr tag = nullptr;
  MemPtrVector mem{};

  // --- function ---
  // Function type.
  TypePtr return_ty = nullptr;
  // function params type list.
  TypePtrVector params{};
};

template <>
bool Type::Is<TY_PRT>() const;

#endif  // !TYPE_GRUAD
