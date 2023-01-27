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

#include <memory>

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

class VarScope {
 public:
  VarScope() = default;
  ObjectPtr& GetVar() { return var; }
  void SetVar(ObjectPtr v) { var = v; }
  TypePtr& GetType() { return tydef; }
  void SetType(TypePtr t) { tydef = t; }

 private:
  ObjectPtr var = nullptr;
  TypePtr tydef = nullptr;
};

class Scope {
 public:
  // get current var scope
  VarScopeMap& GetVarScope() { return vars; }
  // get current tag scope
  TagScopeMap& GetTagScope() { return tags; }

 public:
  // create a scpoe
  static void EnterScope(ScopePtr& next);
  // delete a scope
  static void LevarScope(ScopePtr& next);
  // find a variable by name.
  static VarScopePtr FindVarScope(const String& name);
  // find a tag by name.
  static TypePtr FindTag(const String& name);
  // find a typedef name by name.
  static const TypePtr FindTypedef(const TokenPtr& tok);
  // create a varscope.
  static VarScopePtr& PushVarScope(const String& name);

 private:
  // scope link list.
  ScopePtr next = nullptr;
  // --- C has two black scope; one is for variable and other
  // is for struct tags. ---
  // var scope map.
  VarScopeMap vars = VarScopeMap();
  // tag scope map.
  TagScopeMap tags = TagScopeMap();
};

class Object {
 public:
  // construct a Object object based on kind.
  Object(Objectkind kind, const String& name, const TypePtr& ty)
      : kind(kind), ty(ty), obj_name(name) {}
  // calculate the function local variable offset.
  void OffsetCal();
  // check whether is a local variable
  inline bool IsLocal() { return kind == OB_LOCAL; }
  // check whether is a global variable
  inline bool IsGlobal() { return kind == OB_GLOBAL; }
  // check whether is a global variable or function
  inline bool IsFunction() { return kind == OB_FUNCTION || is_defination == true; }
  // get the object var type.
  inline const TypePtr& GetType() { return ty; }

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

 private:
  friend class CodeGenerator;
  // label the object type
  Objectkind kind = OB_END;
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
};

#endif  // OBJECT_GRUAD