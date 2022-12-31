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
Object* globals;

Object::Object(Objectkind kind, Type* ty, Object** next)
    : kind_(kind)
    , ty_(ty)
    , next_(*next) {
  if (kind == OB_LOCAL && LocalVarFind(ty->name_) != nullptr) {
    ty->name_->ErrorTok("redefined variable.");
  }
  if ((kind == OB_FUNCTION || kind == OB_GLOBAL) &&
      GlobalVarFind(ty->name_) != nullptr) {
    ty->name_->ErrorTok("redefined variable.");
  }
  name_ = ty->name_->GetIdent();
  *next = this;
}

Token* Object::CreateFunction(Token* tok, Type* basety) {
  locals   = nullptr;
  Type* ty = Node::Declarator(&tok, tok, basety);

  Object* fn = new Object(OB_FUNCTION, ty, &globals);
  fn->CreateParamLVar(ty->params_);
  fn->params_   = locals;
  fn->body_     = Node::Program(&tok, tok);
  fn->loc_list_ = locals;
  fn->ty_       = ty;
  return tok;
}

Token* Object::CreateGlobal(Token* tok, Type* basety) {
  bool first = true;

  while (!tok->Equal(";")) {
    if (!first) {
      tok = tok->SkipToken(",");
    }
    first = false;

    Type*   ty = Node::Declarator(&tok, tok, basety);
    Object* gv = new Object(OB_GLOBAL, ty, &globals);
  }
  return tok->SkipToken(";");
}

void Object::CreateParamLVar(Type* param) {
  if (param != nullptr) {
    CreateParamLVar(param->next_);
    Object* v = new Object(OB_LOCAL, param, &locals);
  }
}

bool Object::IsFunction(Token* tok) {
  if (tok->Equal(";")) {
    return false;
  }

  Type* ty  = Node::Declarator(&tok, tok, ty_int);
  bool  ret = (ty->kind_ == TY_FUNC);
  // allocated.
  if (ty != ty_int) {
    if (ty->params_ != nullptr) {
      Type* param_cur = ty->params_;
      while (param_cur && param_cur != ty_int && param_cur != ty_char) {
        ty->params_ = ty->params_->next_;
        delete param_cur;
        param_cur = ty->params_;
      }
    }
    Type::TypeFree(ty);
  }
  return ret;
}

Object* Object::Parse(Token* tok) {
  globals = nullptr;

  while (!tok->IsEof()) {
    Type* basety = Node::Declspec(&tok, tok);

    if (globals->IsFunction(tok)) {
      tok = CreateFunction(tok, basety);
      continue;
    }

    tok = CreateGlobal(tok, basety);
  }
  return globals;
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

Object* Object::LocalVarFind(Token* tok) {
  return locals->Find(tok);
}

Object* Object::GlobalVarFind(Token* tok) {
  return globals->Find(tok);
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