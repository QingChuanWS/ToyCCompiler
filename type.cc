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
#include "type.h"

Type* ty_int = new Type(TY_INT, nullptr);

Type::Type(const Type* ty) {
  kind_      = ty->kind_;
  name_      = ty->name_;
  size_      = ty->size_;
  base_      = ty->base_;
  array_len_ = ty->array_len_;
  return_ty_ = ty->return_ty_;
  params_    = ty->params_;
  next_      = ty->next_;
}

Type::Type(TypeKind kind, Type* base, int len)
    : kind_(kind)
    , name_(nullptr)
    , size_(0)
    , base_(nullptr)
    , array_len_(len)
    , return_ty_(nullptr)
    , params_(nullptr)
    , next_(nullptr) {
  switch (kind_) {
  case TY_INT: size_ = 8; return;
  case TY_PRT:
    base_ = base;
    size_ = 8;
    return;
  case TY_FUNC: return_ty_ = base; return;
  case TY_ARRAY:
    size_ = base->size_ * len;
    base_ = base;
    return;
  default: return;
  }
}

bool Type::IsInteger() {
  return this->kind_ == TY_INT;
}

bool Type::IsPointer() {
  return base_ != nullptr;
}

void Type::TypeFree(Type* head) {
  Type* cur = head;
  while (head && cur != ty_int) {
    head = head->base_;
    delete cur;
    cur = head;
  }
}