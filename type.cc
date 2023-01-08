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

std::shared_ptr<Type> ty_int = std::make_shared<Type>(TY_INT, 8);
std::shared_ptr<Type> ty_char = std::make_shared<Type>(TY_CHAR, 1);

Type* Type::CreatePointerType(Type* base) {
  Type* ty = new Type(TY_PRT, base->size_);
  ty->base_ = base;
  return ty;
}

Type* Type::CreateFunctionType(Type* ret_type, Type* params) {
  Type* ty = new Type(TY_FUNC, ret_type->size_);
  ty->return_ty_ = ret_type;
  ty->params_ = params;
  return ty;
}

Type* Type::CreateArrayType(Type* base, int array_len) {
  Type* ty = new Type(TY_ARRAY, base->size_ * array_len);
  ty->base_ = base;
  return ty;
}

bool Type::IsInteger() { return this->kind_ == TY_INT; }

bool Type::IsFunction() { return this->kind_ == TY_FUNC; }

bool Type::IsPointer() { return base_ != nullptr; }

bool Type::HasName() { return name_ != nullptr; }

void Type::TypeFree(Type* head) {
  Type* cur = head;
  while (cur && cur != ty_int.get() && cur != ty_char.get()) {
    head = head->base_;
    delete cur;
    cur = head;
  }
}