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

#include <memory>

TypePtr ty_int = std::make_shared<Type>(TY_INT, 8);
TypePtr ty_char = std::make_shared<Type>(TY_CHAR, 1);

TypePtr Type::CreatePointerType(TypePtr base) {
  TypePtr ty = std::make_shared<Type>(TY_PRT, base->size);
  ty->base = base;
  return ty;
}

TypePtr Type::CreateFunctionType(TypePtr ret_type, TypePtr params) {
  TypePtr ty = std::make_shared<Type>(TY_FUNC, ret_type->size);
  ty->return_ty = ret_type;
  ty->params = params;
  return ty;
}

TypePtr Type::CreateArrayType(TypePtr base, int array_len) {
  TypePtr ty = std::make_shared<Type>(TY_ARRAY, base->size * array_len);
  ty->base = base;
  return ty;
}

bool Type::IsInteger() { return this->kind == TY_INT; }

bool Type::IsFunction() { return this->kind == TY_FUNC; }

bool Type::IsPointer() { return base != nullptr; }

bool Type::IsArray(){return kind == TY_ARRAY;}

bool Type::HasName() { return name != nullptr; }