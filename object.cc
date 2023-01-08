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

#include <cstdlib>
#include <cstring>

#include "node.h"
#include "tools.h"
#include "type.h"

Object* locals;
Object* globals;

Object* Object::CreateLocalVar(char* name, Type* ty, Object** next) {
  Object* obj = new Object(OB_LOCAL, name, ty);
  obj->name_len_ = strlen(name);
  if (ty->HasName() && ty->name_->FindLocalVar() != nullptr) {
    ty->name_->ErrorTok("redefined variable.");
  }
  obj->next_ = *next;
  *next = obj;
  return obj;
}

Object* Object::CreateGlobalVar(char* name, Type* ty, Object** next) {
  Object* obj = new Object(OB_GLOBAL, name, ty);
  obj->name_len_ = strlen(name);
  if (ty->HasName() && ty->name_->FindGlobalVar() != nullptr) {
    ty->name_->ErrorTok("redefined variable.");
  }
  obj->next_ = *next;
  *next = obj;
  return obj;
}

Object* Object::CreateStringVar(char* name) {
  Type * ty = Type::CreateArrayType(ty_char.get(), strlen(name) + 1);
  Object* obj = CreateGlobalVar(CreateUniqueName(), ty, &globals);
  obj->init_data = name;
  obj->is_string = true;
  return obj;
}

Token* Object::CreateFunction(Token* tok, Type* basety, Object** next) {
  locals = nullptr;
  Type* ty = Node::Declarator(&tok, tok, basety);
  CreateParamVar(ty->params_);

  Object* fn = new Object(OB_FUNCTION, ty->name_->GetIdent(), ty);
  fn->params_ = locals;
  fn->body_ = Node::Program(&tok, tok);
  fn->loc_list_ = locals;
  fn->ty_ = ty;

  fn->next_ = *next;
  *next = fn;
  return tok;
}

void Object::CreateParamVar(Type* param) {
  if (param != nullptr) {
    CreateParamVar(param->next_);
    Object* v = CreateLocalVar(param->name_->GetIdent(), param, &locals);
  }
}

bool Object::IsFunction(Token* tok) {
  if (tok->Equal(";")) {
    return false;
  }
  while (tok->Equal("*")) {
    tok = tok->next_;
  }
  if (tok->kind_ != TK_IDENT) {
    tok->ErrorTok("expected a variable name.");
  }
  tok = tok->next_;
  if (tok->Equal("(")) {
    return true;
  }
  return false;
}

Object* Object::Parse(Token* tok) {
  globals = nullptr;
  while (!tok->IsEof()) {
    Type* basety = Node::Declspec(&tok, tok);
    if (IsFunction(tok)) {
      tok = CreateFunction(tok, basety, &globals);
      continue;
    }
    tok = ParseGlobalVar(tok, basety);
  }
  return globals;
}

Token* Object::ParseGlobalVar(Token* tok, Type* basety) {
  bool first = true;

  while (!tok->Equal(";")) {
    if (!first) {
      tok = tok->SkipToken(",");
    }
    first = false;
    Type* ty = Node::Declarator(&tok, tok, basety);
    Object* gv = CreateGlobalVar(ty->name_->GetIdent(), ty, &globals);
  }
  return tok->SkipToken(";");
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

Object* Object::Find(char* p) {
  for (Object* v = this; v != nullptr; v = v->next_) {
    if (memcmp(v->name_, p, v->name_len_) == 0) {
      return v;
    }
  }
  return nullptr;
}

void Object::FunctionFree() {
  if (IsFunction()) {
    ObjectFree(loc_list_);
    Node::NodeListFree(body_);
  }
}

void Object::ObjectFree(Object* head) {
  Object* cur = head;
  while (cur != nullptr) {
    head = head->next_;
    Type::TypeFree(cur->ty_);
    cur->FunctionFree();
    free(cur->name_);
    delete cur;
    cur = head;
  }
}