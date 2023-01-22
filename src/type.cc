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

TypePtr ty_int = std::make_shared<Type>(TY_INT, 8, 8);
TypePtr ty_char = std::make_shared<Type>(TY_CHAR, 1, 1);

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

TypePtr Type::CreateStructType(StructPtr mem) {
  TypePtr ty = std::make_shared<Type>(TY_STRUCT, 1, 1);
  ty->align = Struct::CalcuAlign(mem);
  ty->size = AlignTo(Struct::CalcuOffset(mem), ty->align);
  ty->mem = mem;
  return ty;
}

bool Type::IsInteger() const { return this->kind == TY_INT; }

bool Type::IsFunction() const { return this->kind == TY_FUNC; }

bool Type::IsPointer() const { return base != nullptr; }

bool Type::IsArray() const { return kind == TY_ARRAY; }

bool Type::IsStruct() const { return kind == TY_STRUCT; }

bool Type::HasName() { return name != nullptr; }

const TokenPtr& Type::GetName() const {
  if (name == nullptr) {
    Error("The type has not name!");
  }
  return name;
}

// get struct member based on token.
StructPtr Type::GetStructMember(TokenPtr tok) {
  for (StructPtr m = mem; m != nullptr; m = m->next) {
    if (tok->Equal(m->name)) {
      return m;
    }
  }
  tok->ErrorTok("no such member.");
  return nullptr;
}