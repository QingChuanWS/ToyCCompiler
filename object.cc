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

#include "node.h"
#include "tools.h"
#include "type.h"

Object* locals;
Object* globals;

Object::Object(Objectkind kind, char* name, Type* ty) : kind_(kind), ty_(ty), name_(name) {}

Object* Object::CreateLocalVar(char* name, Type* ty, Object** next) {
  Object* obj = new Object(OB_LOCAL, name, ty);
  if (ty->HasName() && LocalVarFind(ty->name_) != nullptr) {
    ty->name_->ErrorTok("redefined variable.");
  }
  obj->next_ = *next;
  *next = obj;
  return obj;
}

Object* Object::CreateGlobalVar(char* name, Type* ty, Object** next) {
  Object* obj = new Object(OB_GLOBAL, name, ty);
  if (ty->HasName() && GlobalVarFind(ty->name_) != nullptr) {
    ty->name_->ErrorTok("redefined variable.");
  }
  obj->next_ = *next;
  *next = obj;
  return obj;
}

Token* Object::CreateFunction(Token* tok, Type* basety, Object** next) {
  locals = nullptr;
  Type* ty = Node::Declarator(&tok, tok, basety);

  Object* fn = new Object(OB_FUNCTION, ty->name_->GetIdent(), ty);
  fn->CreateParamLVar(ty->params_);
  fn->params_ = locals;
  fn->body_ = Node::Program(&tok, tok);
  fn->loc_list_ = locals;
  fn->ty_ = ty;

  fn->next_ = *next;
  *next = fn;
  return tok;
}

Token* Object::ParseGlobal(Token* tok, Type* basety) {
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

void Object::CreateParamLVar(Type* param) {
  if (param != nullptr) {
    CreateParamLVar(param->next_);
    Object* v = CreateLocalVar(param->name_->GetIdent(), param, &locals);
  }
}

Object* Object::CreateStringVar(char* name, Type* ty) {
  Object* obj = CreateGlobalVar(CreateUniqueName(), ty, &globals);
  obj->init_data = name;
  obj->is_string = true;
  return obj;
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

    if (globals->IsFunction(tok)) {
      tok = CreateFunction(tok, basety, &globals);
      continue;
    }
    tok = ParseGlobal(tok, basety);
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

Object* Object::LocalVarFind(Token* tok) { return locals->Find(tok); }

Object* Object::GlobalVarFind(Token* tok) { return globals->Find(tok); }

Object* Object::Find(Token* tok) {
  for (Object* v = this; v != nullptr; v = v->next_) {
    if (tok->Equal(v->name_)) {
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