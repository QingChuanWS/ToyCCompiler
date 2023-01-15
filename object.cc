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

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>

#include "node.h"
#include "parser.h"
#include "tools.h"
#include "type.h"
#include "utils.h"

ObjectPtr locals = nullptr;
ObjectPtr globals = nullptr;

ScopePtr scope = nullptr;

void Scope::EnterScope(ScopePtr& next) {
  ScopePtr sc = std::make_shared<Scope>();
  sc->next = next;
  next = sc;
}

void Scope::LevarScope(ScopePtr& next) { next = next->next; }

ObjectPtr Object::CreateVar(Objectkind kind, const String& name, const TypePtr& ty) {
  ObjectPtr obj = std::make_shared<Object>(kind, name, ty);
  VarScope::PushScope(name, obj, scope->vars);
  return obj;
}

ObjectPtr Object::CreateLocalVar(const String& name, const TypePtr& ty, ObjectPtr* next) {
  ObjectPtr obj = CreateVar(OB_LOCAL, name, ty);
  // if (Find(name.c_str()) != nullptr) {
  //   ty->name->ErrorTok("redefined variable.");
  // }
  obj->next = *next;
  *next = obj;
  return obj;
}

ObjectPtr Object::CreateGlobalVar(const String& name, const TypePtr& ty, ObjectPtr* next) {
  ObjectPtr obj = CreateVar(OB_GLOBAL, name, ty);
  // if (ty->HasName() && ty->name->FindVar() != nullptr) {
  //   ty->name->ErrorTok("redefined variable.");
  // }
  obj->next = *next;
  *next = obj;
  return obj;
}

ObjectPtr Object::CreateStringVar(const String& name) {
  TypePtr ty = Type::CreateArrayType(ty_char, name.size());
  ObjectPtr obj = CreateGlobalVar(CreateUniqueName(), ty, &globals);
  obj->init_data = name;
  obj->is_string = true;
  return obj;
}

TokenPtr Object::CreateFunction(TokenPtr tok, TypePtr basety, ObjectPtr* next) {
  locals = nullptr;
  TypePtr ty = Parser::Declarator(&tok, tok, basety);

  // create scope.
  Scope::EnterScope(scope);

  ObjectPtr fn = CreateVar(OB_FUNCTION, ty->name->GetIdent(), ty);
  CreateParamVar(ty->params);
  fn->params = locals;
  fn->body = Parser::Program(&tok, tok);
  fn->loc_list = locals;

  // leave scope.
  Scope::LevarScope(scope);

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

bool Object::IsFuncToks(TokenPtr tok) {
  if (tok->Equal(";")) {
    return false;
  }
  while (tok->Equal("*")) {
    tok = tok->GetNext();
  }
  if (tok->GetKind() != TK_IDENT) {
    tok->ErrorTok("expected a variable name.");
  }
  tok = tok->GetNext();
  if (tok->Equal("(")) {
    return true;
  }
  return false;
}

ObjectPtr Object::Parse(TokenPtr tok) {
  globals = nullptr;
  // enter scope
  Scope::EnterScope(scope);
  while (!tok->IsEof()) {
    TypePtr basety = Parser::Declspec(&tok, tok);
    if (IsFuncToks(tok)) {
      tok = CreateFunction(tok, basety, &globals);
      continue;
    }
    tok = ParseGlobalVar(tok, basety);
  }
  // leave scope.
  Scope::LevarScope(scope);
  return globals;
}

TokenPtr Object::ParseGlobalVar(TokenPtr tok, TypePtr basety) {
  bool first = true;

  while (!tok->Equal(";")) {
    if (!first) {
      tok = tok->SkipToken(",");
    }
    first = false;
    TypePtr ty = Parser::Declarator(&tok, tok, basety);
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

ObjectPtr Object::Find(const char* p) {
  for (ScopePtr sc = scope; sc != nullptr; sc = sc->next) {
    for (VarScopePtr lc_sc = sc->vars; lc_sc != nullptr; lc_sc = lc_sc->next) {
      if (memcmp(lc_sc->name.c_str(), p, lc_sc->name.size()) == 0) {
        return lc_sc->var;
      }
    }
  }
  return nullptr;
}