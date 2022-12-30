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

#include "object.h"

#include "node.h"
#include "type.h"

#include <cstdlib>

Object* locals;

Object::Object(Token** rest, Token* tok)
    : kind_(OB_FUNCTION) {
  kind_ = OB_FUNCTION;

  ty_ = Node::Declspec(&tok, tok);
  ty_      = Node::Declarator(&tok, tok, ty_);

  locals = nullptr;
  name_  = ty_->name_->GetIdent();
  CreateParamLVar(ty_->params_);
  params_ = locals;

  body_     = Node::Program(rest, tok);
  loc_list_ = locals;
}

void Object::CreateParamLVar(Type* param) {
  if (param != nullptr) {
    CreateParamLVar(param->next_);
    Object* v = new Object(OB_LOCAL, param->name_->GetIdent(), &locals, param);
  }
}

Object* Object::Parse(Token* tok) {
  Object  head = Object();
  Object* cur  = &head;

  while (!tok->IsEof()) {
    cur->next_ = new Object(&tok, tok);
    cur        = cur->next_;
  }
  return head.next_;
}

void Object::OffsetCal() {
  for (Object* fn = this; fn != nullptr; fn = fn->next_) {
    int offset = 0;
    for (Object* cur = fn->loc_list_; cur != nullptr; cur = cur->next_) {
      offset += cur->ty_->size_;
      cur->offset_ = offset;
    }
    fn->stack_size_ = AlignTo(offset, 16);
  }
}

Object* Object::Find(Token* tok) {
  for (Object* v = this; v != nullptr; v = v->next_) {
    if (tok->Equal(v->name_)) {
      return v;
    }
  }
  return nullptr;
}

void Object::ObjectFree(Object* head) {
  Object* cur = head;
  while (cur != nullptr) {
    head = head->next_;
    Type::TypeFree(cur->ty_);
    if (cur->IsFunction()) {
      ObjectFree(cur->loc_list_);
      Node::NodeListFree(cur->body_);
    }
    free(cur->name_);
    delete cur;
    cur = head;
  }
}