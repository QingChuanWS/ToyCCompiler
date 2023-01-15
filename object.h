/*
 * This project is exclusively owned by QingChuanWS and shall not be used for
 * commercial and profitting purpose without QingChuanWS's permission.
 *
 * @Author: bingshan45@163.com
 * Github: https://github.com/QingChuanWS
 * @Description:
 *
 * Copyright (c) 2023 by QingChuanWS, All Rights Reserved.
 */

#ifndef OBJECT_GRUAD
#define OBJECT_GRUAD

#include <memory>

#include "tools.h"
#include "type.h"
#include "utils.h"

enum Objectkind {
  OB_LOCAL,
  OB_GLOBAL,
  OB_FUNCTION,
  OB_END,
};

class VarScope {
public:
  VarScope(String name, ObjectPtr var) : name(name), var(var) {}

  static VarScopePtr PushScope(String name, ObjectPtr var, VarScopePtr& scope) {
    VarScopePtr sc = std::make_shared<VarScope>(name, var);
    sc->next = scope;
    scope = sc;
    return sc;
  }

 private:
  friend class Object;
  VarScopePtr next = nullptr;
  String name = String();
  ObjectPtr var = nullptr;
};

class Scope {
 public:
  static void EnterScope(ScopePtr& next);
  static void LevarScope(ScopePtr& next);

 private:
  friend class Object;
  ScopePtr next = nullptr;
  VarScopePtr vars = nullptr;
};

class Object {
 public:
  // construct a Object object based on kind.
  Object(Objectkind kind, const String& name, const TypePtr& ty)
      : kind(kind), ty(ty), obj_name(name) {}
  // calculate the function local variable offset.
  void OffsetCal();
  // check whether is a local variable
  bool IsLocal() { return kind == OB_LOCAL; }
  // check whether is a global variable
  bool IsGlobal() { return kind == OB_GLOBAL; }
  // check whether is a global variable or function
  bool IsFunction() { return kind == OB_FUNCTION; }
  // get the object var type.
  const TypePtr& GetType() const { return ty; }
  // find a variable by name.
  static ObjectPtr Find(const char* p);

 public:
  // create variable.
  static ObjectPtr CreateVar(Objectkind kind, const String& name, const TypePtr& ty);
  // create global varibal
  static ObjectPtr CreateGlobalVar(const String& name, const TypePtr& ty, ObjectPtr* next);
  // create local varibal
  static ObjectPtr CreateLocalVar(const String& name, const TypePtr& ty, ObjectPtr* next);
  // create a function based on token list.
  static TokenPtr CreateFunction(TokenPtr tok, TypePtr basety, ObjectPtr* next);
  // create a string literal variable
  static ObjectPtr CreateStringVar(const String& name);
  // create function parameter list.
  static void CreateParamVar(TypePtr param);
  // parsing token list and generate AST.
  static ObjectPtr Parse(TokenPtr tok);
  // Lookahead tokens and returns true if a given token is a start
  // of a function definition or declaration.
  static bool IsFuncToks(TokenPtr tok);
  // create global variable list based on token list.
  static TokenPtr ParseGlobalVar(TokenPtr tok, TypePtr basety);

  friend class CodeGenerator;

 private:
  // label the object type
  Objectkind kind = OB_END;
  // for object list
  ObjectPtr next = nullptr;
  // variable name
  String obj_name = String();
  // Type
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
};

#endif  // OBJECT_GRUAD