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

#include "type.h"

class Node;

enum Objectkind {
  OB_LOCAL,
  OB_GLOBAL,
  OB_FUNCTION,
  OB_END,
};

class Object {
 public:
  explicit Object() = default;
  // construct a Object object based on kind.
  Object(Objectkind kind, char* name, Type* ty) : kind_(kind), ty_(ty), name_(name) {}
  // calculate the function local variable offset.
  void OffsetCal();
  // check whether is a local variable
  bool IsLocal() { return kind_ == OB_LOCAL; }
  // check whether is a global variable
  bool IsGlobal() { return kind_ == OB_GLOBAL; }
  // check whether is a global variable or function
  bool IsFunction() { return kind_ == OB_FUNCTION; }
  // find a token name in object list.
  Object* Find(char* p);

 public:
  // create global varibal
  static Object* CreateGlobalVar(char* name, Type* ty, Object** next);
  // create local varibal
  static Object* CreateLocalVar(char* name, Type* ty, Object** next);
  // create a function based on token list.
  static Token* CreateFunction(Token* tok, Type* basety, Object** next);
  // create a string literal variable
  static Object* CreateStringVar(char* p);
  // create function parameter list.
  static void CreateParamVar(Type* param);
  // parsing token list and generate AST.
  static Object* Parse(Token* tok);
  // Lookahead tokens and returns true if a given token is a start
  // of a function definition or declaration.
  static bool IsFunction(Token* tok);
  // create global variable list based on token list.
  static Token* ParseGlobalVar(Token* tok, Type* basety);
  // free the object list.
  static void ObjectFree(Object* head);

  friend class Node;
  friend class CodeGenerator;

 private:
  // free function
  void FunctionFree();
  // label the object type
  Objectkind kind_ = OB_END;
  // for object list
  Object* next_ = nullptr;
  // variable name
  char* name_ = nullptr;
  // variable name len
  int name_len_ = 0;
  // Type
  Type* ty_ = nullptr;

  // local variable offset
  int offset_ = 0;

  // Global Variable
  char* init_data = nullptr;
  // whether is a stirng
  bool is_string = false;

  // function parameter
  Object* params_ = nullptr;
  // function body
  Node* body_ = nullptr;
  // function variable list
  Object* loc_list_ = nullptr;
  // function variable's stack size
  int stack_size_ = 0;
};

extern Object* locals;
extern Object* globals;

#endif  // OBJECT_GRUAD