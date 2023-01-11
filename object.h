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
  Object(Objectkind kind, String name, TypePtr ty) : kind(kind), ty(ty), obj_name(std::move(name)) {}
  // deconstructor.
  // ~Object();
  // calculate the function local variable offset.
  void OffsetCal();
  // check whether is a local variable
  bool IsLocal() { return kind == OB_LOCAL; }
  // check whether is a global variable
  bool IsGlobal() { return kind == OB_GLOBAL; }
  // check whether is a global variable or function
  bool IsFunction() { return kind == OB_FUNCTION; }
  // find a token name in object list.
  static ObjectPtr Find(ObjectPtr root, char* p);

 public:
  // create global varibal
  static ObjectPtr CreateGlobalVar(String name, TypePtr ty, ObjectPtr* next);
  // create local varibal
  static ObjectPtr CreateLocalVar(String name, TypePtr ty, ObjectPtr* next);
  // create a function based on token list.
  static TokenPtr CreateFunction(TokenPtr tok, TypePtr basety, ObjectPtr* next);
  // create a string literal variable
  static ObjectPtr CreateStringVar(String& name);
  // create function parameter list.
  static void CreateParamVar(TypePtr param);
  // parsing token list and generate AST.
  static ObjectPtr Parse(TokenPtr tok);
  // Lookahead tokens and returns true if a given token is a start
  // of a function definition or declaration.
  static bool IsFunction(TokenPtr tok);
  // create global variable list based on token list.
  static TokenPtr ParseGlobalVar(TokenPtr tok, TypePtr basety);

  friend class Node;
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