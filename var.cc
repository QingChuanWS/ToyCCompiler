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

#include "var.h"

// All local variable instances created during parsing are
// accumulated to this list.
Var* locals;

Var* Var::Find(Token* tok) {
  for (Var* v = this; v != nullptr; v = v->next_) {
    if (tok->Equal(v->name_)) {
      return v;
    }
  }
  return nullptr;
}

void Var::VarFree(Var* head){
  Var* cur = head;
  while(cur != nullptr){
    head = head->next_;
    free(cur->name_);
    delete cur;
    cur = head;
  }
}
