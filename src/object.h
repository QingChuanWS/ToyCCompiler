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
#include "utils.h"

enum Objectkind {
  OB_LOCAL,
  OB_GLOBAL,
  OB_FUNCTION,
  OB_END,
};
class Object {
 public:
  // construct a Object object based on kind.
  Object(Objectkind kind, const String& name, const TypePtr& ty)
      : kind(kind), obj_name(name), ty(ty) {}
  // calculate the function local variable offset.
  void OffsetCal();
  // check whether is a local variable
  inline bool IsLocal() const { return kind == Objectkind::OB_LOCAL; }
  // check whether is a global variable
  inline bool IsGlobal() const { return kind == Objectkind::OB_GLOBAL; }
  // check whether is a global variable or function
  inline bool IsFunction() const {
    return kind == Objectkind::OB_FUNCTION || is_defination == true;
  }
  // get the object var type.
  inline const TypePtr& GetType() const { return ty; }

  // create variable.
  static ObjectPtr CreateVar(Objectkind kind, const String& name, const TypePtr& ty);
  // create global varibal
  static ObjectPtr CreateGlobalVar(const String& name, const TypePtr& ty, ObjectPtr* next);
  // create local varibal
  static ObjectPtr CreateLocalVar(const String& name, const TypePtr& ty, ObjectPtr* next);
  // create a function based on token list.
  static TokenPtr CreateFunction(TokenPtr tok, TypePtr basety, VarAttrPtr attr, ObjectPtr* next);
  // create a string literal variable
  static ObjectPtr CreateStringVar(const String& name);
  // create function parameter list.
  static void CreateParamVar(TypeVector& param);
  // parsing token list and generate AST.
  static ObjectPtr Parse(TokenPtr tok);
  // Lookahead tokens and returns true if a given token is a start
  // of a function definition or declaration.
  static bool IsFuncToks(TokenPtr tok);
  // create global variable list based on token list.
  static TokenPtr ParseGlobalVar(TokenPtr tok, TypePtr basety);

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
  ObjectPtr params = nullptr;
  // function body
  NodePtr body = nullptr;
  // function variable list
  ObjectPtr loc_list = nullptr;
  // function variable's stack size
  int stack_size = 0;
  // function only have defination.
  bool is_defination = false;
  // function is a static function
  bool is_static = false;
};

#endif  // OBJECT_GRUAD