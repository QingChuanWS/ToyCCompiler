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
#include "function.h"

#include "type.h"
#include <cstdlib>

Function::Function(Token** rest, Token* tok) {
  Type* ty = Node::Declspec(&tok, tok);
  ty       = Node::Declarator(&tok, tok, ty);

  locals = nullptr;
  name_  = ty->name_->GetIdent();

  body_     = Node::Program(rest, tok);
  var_list_ = locals;
  delete ty;
}

Function* Function::Parse(Token* tok) {
  Function  head = Function();
  Function* cur  = &head;

  while (!tok->IsEof()) {
    cur->next_ = new Function(&tok, tok);
    cur        = cur->next_;
  }
  return head.next_;
}

void Function::OffsetCal(Function* prog) {
  for (Function* fn = prog; fn != nullptr; fn = fn->next_) {
    int offset = 0;
    for (Var* cur = fn->var_list_; cur != nullptr; cur = cur->next_) {
      offset += 8;
      cur->offset_ = offset;
    }
    fn->stack_size_ = AlignTo(offset, 16);
  }
}

void Function::FunctionFree(Function* head) {
  Function* cur = head;
  while (cur != nullptr) {
    head = head->next_;
    Node::NodeListFree(cur->body_);
    Var::VarFree(cur->var_list_);
    free(cur->name_);
    delete cur;
    cur = head;
  }
  delete ty_int;
}