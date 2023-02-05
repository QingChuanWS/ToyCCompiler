/*
 * This project is exclusively owned by QingChuanWS and shall not be used for
 * commercial and profitting purpose without QingChuanWS's permission.
 *
 * @ Author: bingshan45@163.com
 * @ Github: https://github.com/QingChuanWS
 * @ Description:
 *
 * Copyright (c) 2023 by QingChuanWS, All Rights Reserved.
 */

#include "scope.h"

// All variable or tag scope instance are accumulated to this list.
ScopePtr scope = nullptr;

void Scope::EnterScope(ScopePtr& next) {
  ScopePtr sc = std::make_shared<Scope>();
  sc->next = next;
  next = sc;
}

void Scope::LevarScope(ScopePtr& next) { next = next->next; }

VarScopePtr& Scope::PushVarScope(const String& name) {
  scope->vars[name] = std::make_shared<VarScope>();
  return scope->vars[name];
}

VarScopePtr Scope::FindVarScope(const String& name) {
  for (ScopePtr sc = scope; sc != nullptr; sc = sc->next) {
    auto v = sc->vars.find(name);
    if (v != sc->vars.end()) {
      return v->second;
    }
  }
  return nullptr;
}

TypePtr Scope::FindTag(const String& name) {
  for (ScopePtr sc = scope; sc != nullptr; sc = sc->next) {
    auto t = sc->tags.find(name);
    if (t != sc->tags.end()) {
      return t->second;
    }
  }
  return nullptr;
}

const TypePtr Scope::FindTypedef(const TokenPtr& tok) {
  if (tok->Is<TK_IDENT>()) {
    VarScopePtr v = FindVarScope(tok->GetIdent());
    if (v != nullptr) {
      return v->tydef;
    }
  }
  return nullptr;
}