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

#ifndef OBJECT_GRUAD
#define OBJECT_GRUAD

#include "parser.h"
#include "tools.h"
#include "type.h"

enum Objectkind {
  OB_LOCAL,
  OB_GLOBAL,
  OB_FUNCTION,
  OB_END,
};

struct FuncAttr {
  // function variable's stack size
  int stack_size;
  // function only have defination.
  bool is_defination;
  // function is a static function
  bool is_static;
};

class Object {
 public:
  // construct a Object object based on kind.
  Object(Objectkind kind, const String& name, const TypePtr& ty)
      : kind(kind), obj_name(name), ty(ty) {}
  // calculate the function local variable offset.
  static void OffsetCal(ObjectList program);
  // whether the object is a <kind> object
  template <Objectkind kind>
  bool Is() {
    return this->kind == kind;
  }
  // get the object var type.
  inline const TypePtr& GetType() const { return ty; }

  // create variable.
  static ObjectPtr CreateVar(Objectkind kind, const String& name, const TypePtr& ty);
  // create global varibal
  static ObjectPtr CreateGlobalVar(const String& name, const TypePtr& ty, ObjectList& globals);
  // create local varibal
  static ObjectPtr CreateLocalVar(const String& name, const TypePtr& ty, ObjectList& locals);
  // create a function based on token list.
  static ObjectPtr CreateFunction(String func_name, TypePtr func_type, ObjectList&& params,
                                  ObjectList&& locals, NodePtr func_body, FuncAttr f_attr);
  // create a string literal variable
  static ObjectPtr CreateStringVar(const String& name, ObjectList& globals);

 private:
  friend class CodeGenerator;
  // label the object type
  Objectkind kind = Objectkind::OB_END;
  // for object list
  ObjectPtr next = nullptr;
  // variable name
  String obj_name = String();
  // Object type
  TypePtr ty = nullptr;

  // local variable offset
  int offset = 0;

  // Global Variable
  String init_data = String();
  // whether is a stirng
  bool is_string = false;

  // function parameter
  ObjectList params{};
  // function body
  NodePtr body = nullptr;
  // function variable list
  ObjectList loc_list{};
  // func attribution
  FuncAttr func_attr{0, false, false};
};

template <>
bool Object::Is<OB_FUNCTION>();

#endif  // OBJECT_GRUAD