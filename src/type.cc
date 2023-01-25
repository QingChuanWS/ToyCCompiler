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
#include "type.h"

#include <cstddef>
#include <memory>

#include "struct.h"
#include "tools.h"
#include "utils.h"

TypePtr ty_long = std::make_shared<Type>(TY_CHAR, 8, 8);
TypePtr ty_int = std::make_shared<Type>(TY_INT, 4, 4);
TypePtr ty_short = std::make_shared<Type>(TY_SHORT, 2, 2);
TypePtr ty_char = std::make_shared<Type>(TY_CHAR, 1, 1);
TypePtr ty_void = std::make_shared<Type>(TY_VOID, 1, 1);

bool Type::IsInteger() const {
  return kind == TY_INT || kind == TY_CHAR || kind == TY_SHORT || kind == TY_LONG;
}

TypePtr Type::CreatePointerType(TypePtr base) {
  TypePtr ty = std::make_shared<Type>(TY_PRT, 8, 8);
  ty->base = base;
  return ty;
}

TypePtr Type::CreateFunctionType(TypePtr ret_type, TypePtr params) {
  TypePtr ty = std::make_shared<Type>(TY_FUNC, ret_type->size, 0);
  ty->return_ty = ret_type;
  ty->params = params;
  return ty;
}

TypePtr Type::CreateArrayType(TypePtr base, int array_len) {
  TypePtr ty = std::make_shared<Type>(TY_ARRAY, base->size * array_len, base->align);
  ty->base = base;
  return ty;
}

TypePtr Type::CreateStructType(MemberPtr mem) {
  TypePtr ty = std::make_shared<Type>(TY_STRUCT, 1, 1);
  ty->align = Member::CalcuStructAlign(mem);
  ty->size = AlignTo(Member::CalcuStructOffset(mem), ty->align);
  ty->mem = mem;
  return ty;
}

TypePtr Type::CreateUnionType(MemberPtr mem) {
  TypePtr ty = std::make_shared<Type>(TY_UNION, 1, 1);
  for (MemberPtr m = mem; m != nullptr; m = m->next) {
    if (ty->align < m->ty->align) {
      ty->align = m->ty->align;
    }
    if (ty->size < m->ty->size) {
      ty->size = m->ty->size;
    }
  }
  ty->size = AlignTo(ty->size, ty->align);
  ty->mem = mem;
  return ty;
}

const TokenPtr& Type::GetName() const {
  if (name == nullptr) {
    Error("The type has not name!");
  }
  return name;
}

// get struct member based on token.
MemberPtr Type::GetStructMember(TokenPtr tok) {
  for (MemberPtr m = mem; m != nullptr; m = m->next) {
    if (tok->Equal(m->name)) {
      return m;
    }
  }
  tok->ErrorTok("no such member.");
  return nullptr;
}