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
#include <vector>

#include "context.h"
#include "node.h"
#include "parser.h"
#include "scope.h"
#include "token.h"
#include "tools.h"
#include "type.h"
#include "utils.h"

ObjectPtr Object::CreateVar(Objectkind kind, const String& name, const TypePtr& ty) {
  auto obj = std::make_shared<Object>(kind, name, ty);
  Scope::PushVarScope(name)->var = obj;
  return obj;
}

ObjectPtr Object::CreateLocalVar(const String& name, const TypePtr& ty, ObjectList& locals) {
  ObjectPtr obj = CreateVar(Objectkind::OB_LOCAL, name, ty);
  // if (Find(name.c_str()) != nullptr) {
  //   ty->name->ErrorTok("redefined variable.");
  // }
  locals.push_back(obj);
  return obj;
}

ObjectPtr Object::CreateGlobalVar(const String& name, const TypePtr& ty, ObjectList& globals) {
  ObjectPtr obj = CreateVar(Objectkind::OB_GLOBAL, name, ty);
  // if (ty->HasName() && ty->name->FindVar() != nullptr) {
  //   ty->name->ErrorTok("redefined variable.");
  // }
  globals.push_back(obj);
  return obj;
}

ObjectPtr Object::CreateStringVar(const String& name, ObjectList& globals) {
  TypePtr ty = Type::CreateArrayType(ty_char, name.size());
  ObjectPtr obj = CreateGlobalVar(CreateUniqueName(), ty, globals);
  obj->init_data = name;
  obj->is_string = true;
  return obj;
}

TokenPtr Object::CreateFunction(TokenPtr tok, TypePtr basety, VarAttrPtr attr, ASTree& ct) {
  TypePtr ty = Parser::Declarator(&tok, tok, basety, ct);
  ObjectPtr fn = CreateVar(Objectkind::OB_FUNCTION, ty->name->GetIdent(), ty);

  ct.locals.clear();
  // function declaration
  if (tok->Equal(";")) {
    fn->is_defination = true;
    tok = tok->SkipToken(";");
    return tok;
  }
  fn->is_static = attr->is_static;
  cur_fn = fn;
  // create scope.
  Scope::EnterScope(scope);

  // funtion defination.
  for (auto i = ty->params.rbegin(); i != ty->params.rend(); ++i) {
    ObjectPtr v = CreateLocalVar((*i)->name->GetIdent(), *i, ct.locals);
  }

  fn->params = ct.locals;
  fn->body = Parser::Program(&tok, tok, ct);
  fn->loc_list = ct.locals;
  ct.globals.push_back(fn);

  // leave scope.
  Scope::LevarScope(scope);

  Node::UpdateGotoLabel();
  return tok;
}

void Object::OffsetCal(ObjectList program) {
  for (auto fn : program) {
    if (!fn->Is<OB_FUNCTION>()) {
      continue;
    }

    int ofs = 0;
    for (auto v = fn->loc_list.rbegin(); v != fn->loc_list.rend(); v++) {
      ofs += (*v)->ty->size;
      ofs = AlignTo(ofs, (*v)->ty->align);
      (*v)->offset = ofs;
    }
    fn->stack_size = AlignTo(ofs, 16);
  }
}

template <>
bool Object::Is<OB_FUNCTION>() {
  return this->kind == Objectkind::OB_FUNCTION || this->is_defination == true;
}
