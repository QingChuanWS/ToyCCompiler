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

ObjectPtr Object::CreateFunction(String func_name, TypePtr func_type, ObjectList&& params,
                                 ObjectList&& locals, NodePtr func_body, FuncAttr f_attr) {
  ObjectPtr fn = CreateVar(Objectkind::OB_FUNCTION, func_name, func_type);

  fn->func_attr = f_attr;
  fn->params = std::forward<ObjectList>(params);
  fn->body = func_body;
  fn->loc_list = std::forward<ObjectList>(locals);

  return fn;
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
    fn->func_attr.stack_size = AlignTo(ofs, 16);
  }
}

template <>
bool Object::Is<OB_FUNCTION>() {
  return this->kind == Objectkind::OB_FUNCTION || this->func_attr.is_defination == true;
}
