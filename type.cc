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

bool Type::IsInteger() {
  return this->kind_ == TY_INT;
}

bool Type::IsPointer() {
  return base_ != nullptr;
}

void Type::TypeFree(Type* head){
  Type* cur = head;
  while(cur != ty_int){
    head = head->base_;
    delete cur;
    cur = head;
  }
}