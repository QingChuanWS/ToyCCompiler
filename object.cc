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

ObjectPtr Object::CreateLocalVar(const String& name, TypePtr ty, ObjectPtr* next) {
  ObjectPtr obj = std::make_shared<Object>(OB_LOCAL, name, ty);
  if (ty->HasName() && ty->name->FindLocalVar() != nullptr) {
    ty->name->ErrorTok("redefined variable.");
  }
  obj->next = *next;
  *next = obj;
  return obj;
}

ObjectPtr Object::CreateGlobalVar(const String& name, TypePtr ty, ObjectPtr* next) {
  ObjectPtr obj = std::make_shared<Object>(OB_GLOBAL, name, ty);
  ;
  if (ty->HasName() && ty->name->FindGlobalVar() != nullptr) {
    ty->name->ErrorTok("redefined variable.");
  }
  obj->next = *next;
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
  CreateParamVar(ty->params);

  ObjectPtr fn = std::make_shared<Object>(OB_FUNCTION, ty->name->GetIdent(), ty);
  fn->params = locals;
  fn->body = Node::Program(&tok, tok);
  fn->loc_list = locals;
  fn->ty = ty;

  fn->next = *next;
  *next = fn;
  return tok;
}

void Object::CreateParamVar(TypePtr param) {
  if (param != nullptr) {
    CreateParamVar(param->next);
    ObjectPtr v = CreateLocalVar(param->name->GetIdent(), param, &locals);
  }
}

bool Object::IsFunction(TokenPtr tok) {
  if (tok->Equal(";")) {
    return false;
  }
  while (tok->Equal("*")) {
    tok = tok->next;
  }
  if (tok->kind != TK_IDENT) {
    tok->ErrorTok("expected a variable name.");
  }
  tok = tok->next;
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
    ObjectPtr gv = CreateGlobalVar(ty->name->GetIdent(), ty, &globals);
  }
  return tok->SkipToken(";");
}

void Object::OffsetCal() {
  for (Object* fn = this; fn != nullptr; fn = fn->next.get()) {
    int offset = 0;
    for (ObjectPtr cur = fn->loc_list; cur != nullptr; cur = cur->next) {
      offset += cur->ty->size;
      cur->offset = offset;
    }
    fn->stack_size = AlignTo(offset, 16);
  }
}

ObjectPtr Object::Find(ObjectPtr root, char* p) {
  for (ObjectPtr v = root; v != nullptr; v = v->next) {
    if (memcmp(v->obj_name.c_str(), p, v->obj_name.size()) == 0) {
      return v;
    }
  }
  return nullptr;
}