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

#include "object.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>

#include "node.h"
#include "parser.h"
#include "scope.h"
#include "token.h"
#include "tools.h"
#include "type.h"
#include "utils.h"

// All local variable instance created during parsing are accumulated to this list.
// each function has self local variable.
ObjectPtr locals = nullptr;
// Likewise, global variable are accumulated to this list.
ObjectPtr globals = nullptr;
// current parsering function.
ObjectPtr cur_fn = nullptr;

ObjectPtr Object::CreateVar(Objectkind kind, const String& name, const TypePtr& ty) {
  auto obj = std::make_shared<Object>(kind, name, ty);
  Scope::PushVarScope(name)->var = obj;
  return obj;
}

ObjectPtr Object::CreateLocalVar(const String& name, const TypePtr& ty, ObjectPtr* next) {
  ObjectPtr obj = CreateVar(Objectkind::OB_LOCAL, name, ty);
  // if (Find(name.c_str()) != nullptr) {
  //   ty->name->ErrorTok("redefined variable.");
  // }
  obj->next = *next;
  *next = obj;
  return obj;
}

ObjectPtr Object::CreateGlobalVar(const String& name, const TypePtr& ty, ObjectPtr* next) {
  ObjectPtr obj = CreateVar(Objectkind::OB_GLOBAL, name, ty);
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

TokenPtr Object::CreateFunction(TokenPtr tok, TypePtr basety, VarAttrPtr attr, ObjectPtr* next) {
  TypePtr ty = Parser::Declarator(&tok, tok, basety);
  ObjectPtr fn = CreateVar(Objectkind::OB_FUNCTION, ty->name->GetIdent(), ty);

  // function declaration
  if (tok->Equal(";")) {
    fn->is_defination = true;
    tok = tok->SkipToken(";");
    return tok;
  }
  fn->is_static = attr->is_static;
  cur_fn = fn;
  locals = nullptr;
  // create scope.
  Scope::EnterScope(scope);

  // funtion defination.
  CreateParamVar(ty->params);
  fn->params = locals;
  fn->body = Parser::Program(&tok, tok);
  fn->loc_list = locals;
  fn->next = *next;
  *next = fn;
  // leave scope.
  Scope::LevarScope(scope);
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
    tok = Token::GetNext<1>(tok);
  }
  if (!tok->Is<TK_IDENT>()) {
    tok->ErrorTok("expected a variable name.");
  }
  tok = Token::GetNext<1>(tok);
  if (tok->Equal("(")) {
    return true;
  }
  return false;
}

ObjectPtr Object::Parse(TokenPtr tok) {
  globals = nullptr;
  // enter scope
  Scope::EnterScope(scope);
  while (!tok->Is<TK_EOF>()) {
    auto attr = std::make_shared<VarAttr>();
    TypePtr basety = Parser::Declspec(&tok, tok, attr);

    if (attr->is_typedef) {
      TypePtr ty_list = Parser::TypedefDecl(&tok, tok, basety);
      for (TypePtr t = ty_list; t; t = t->next) {
        Scope::PushVarScope(t->name->GetIdent())->tydef = t;
      }
      continue;
    }

    if (IsFuncToks(tok)) {
      tok = CreateFunction(tok, basety, attr, &globals);
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
    if (!fn->IsFunction()) {
      continue;
    }

    int ofs = 0;
    for (ObjectPtr v = fn->loc_list; v != nullptr; v = v->next) {
      ofs += v->ty->size;
      ofs = AlignTo(ofs, v->ty->align);
      v->offset = ofs;
    }
    fn->stack_size = AlignTo(ofs, 16);
  }
}
