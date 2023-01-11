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
#include <memory>

#include "node.h"
#include "tools.h"
#include "type.h"

ObjectPtr locals;
ObjectPtr globals;

ObjectPtr Object::CreateLocalVar(String name, TypePtr ty, ObjectPtr* next) {
  ObjectPtr obj = std::make_shared<Object>(OB_LOCAL, name, ty);
  if (ty->HasName() && ty->name_->FindLocalVar() != nullptr) {
    ty->name_->ErrorTok("redefined variable.");
  }
  obj->next_ = *next;
  *next = obj;
  return obj;
}

ObjectPtr Object::CreateGlobalVar(String name, TypePtr ty, ObjectPtr* next) {
  ObjectPtr obj =  std::make_shared<Object>(OB_GLOBAL, name, ty);;
  if (ty->HasName() && ty->name_->FindGlobalVar() != nullptr) {
    ty->name_->ErrorTok("redefined variable.");
  }
  obj->next_ = *next;
  *next = obj;
  return obj;
}

ObjectPtr Object::CreateStringVar(String& name) {
  TypePtr ty = Type::CreateArrayType(ty_char, name.size());
  ObjectPtr obj = CreateGlobalVar(CreateUniqueName(), ty, &globals);
  obj->init_data = std::move(name);
  obj->is_string = true;
  return obj;
}

TokenPtr Object::CreateFunction(TokenPtr tok, TypePtr basety, ObjectPtr* next) {
  locals = nullptr;
  TypePtr ty = Node::Declarator(&tok, tok, basety);
  CreateParamVar(ty->params_);

  ObjectPtr fn =  std::make_shared<Object>(OB_FUNCTION, ty->name_->GetIdent(), ty);
  fn->params_ = locals;
  fn->body_ = Node::Program(&tok, tok);
  fn->loc_list_ = locals;
  fn->ty_ = ty;

  fn->next_ = *next;
  *next = fn;
  return tok;
}

void Object::CreateParamVar(TypePtr param) {
  if (param != nullptr) {
    CreateParamVar(param->next_);
    ObjectPtr v = CreateLocalVar(param->name_->GetIdent(), param, &locals);
  }
}

bool Object::IsFunction(TokenPtr tok) {
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

ObjectPtr Object::Parse(TokenPtr tok) {
  globals = nullptr;
  while (!tok->IsEof()) {
    TypePtr basety = Node::Declspec(&tok, tok);
    if (IsFunction(tok)) {
      tok = CreateFunction(tok, basety, &globals);
      continue;
    }
    tok = ParseGlobalVar(tok, basety);
  }
  return globals;
}

TokenPtr Object::ParseGlobalVar(TokenPtr tok, TypePtr basety) {
  bool first = true;

  while (!tok->Equal(";")) {
    if (!first) {
      tok = tok->SkipToken(",");
    }
    first = false;
    TypePtr ty = Node::Declarator(&tok, tok, basety);
    ObjectPtr gv = CreateGlobalVar(ty->name_->GetIdent(), ty, &globals);
  }
  return tok->SkipToken(";");
}

void Object::OffsetCal() {
  for (Object* fn = this; fn != nullptr; fn = fn->next_.get()) {
    int offset = 0;
    for (ObjectPtr cur = fn->loc_list_; cur != nullptr; cur = cur->next_) {
      offset += cur->ty_->size_;
      cur->offset_ = offset;
    }
    fn->stack_size_ = AlignTo(offset, 16);
  }
}

ObjectPtr Object::Find(ObjectPtr root, char* p) {
  for (ObjectPtr v = root; v != nullptr; v = v->next_) {
    if (memcmp(v->name_.c_str(), p, v->name_.size()) == 0) {
      return v;
    }
  }
  return nullptr;
}