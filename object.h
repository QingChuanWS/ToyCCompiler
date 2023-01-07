/*
 * This project is exclusively owned by QingChuanWS and shall not be used for
 * commercial and profitting purpose without QingChuanWS's permission.
 *
 * @Author: bingshan45@163.com
 * Github: https://github.com/QingChuanWS
 * @Description: Object class defination.
 *
 * Copyright (c) 2022 by QingChuanWS, All Rights Reserved.
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
  explicit Object()
      : kind_(OB_END)
      , next_(nullptr)
      , name_(nullptr)
      , body_(nullptr)
      , ty_(nullptr) {}
  
  // construct a Object object based on kind.
  Object(Objectkind kind, char* name, Type* ty);
  // create a string literal variable
  static Object* CreateStringVar(char* p, Type* ty);
  // create a function based on token list.
  static Token* CreateFunction(Token* tok, Type* basety, Object** next);
  // create global variable list based on token list.
  static Token* ParseGlobal(Token* tok, Type* basety);
  // create global varibal
  static Object* CreateGlobalVar(char* name, Type* ty, Object** next);
  // create local varibal
  static Object* CreateLocalVar(char* name, Type* ty, Object** next);
  // parsing token list and generate AST.
  static Object* Parse(Token* tok);
  // calculate the function local variable offset.
  void OffsetCal();
  // create function parameter list.
  void CreateParamLVar(Type* param);
  // check whether is a local variable
  bool IsLocal() { return kind_ == OB_LOCAL; }
  // check whether is a global variable
  bool IsGlobal() { return kind_ == OB_GLOBAL; }
  // check whether is a global variable or function
  bool IsFunction() { return kind_ == OB_FUNCTION; }
  // Lookahead tokens and returns true if a given token is a start
  // of a function definition or declaration.
  bool IsFunction(Token* tok);
  // free the object list.
  static void ObjectFree(Object* head);
  // find a token name in local variable list.
  static Object* LocalVarFind(Token* tok);
  // find a token name in global variable list.
  static Object* GlobalVarFind(Token* tok);
  // find a token name in object list.
  Object* Find(Token* tok);

  friend class Node;
  friend class CodeGenerator;

 private:
  // free function
  void FunctionFree();
  // label the object type
  Objectkind kind_;
  // for object list
  Object* next_=nullptr;
  // variable name
  char* name_=nullptr;
  // Type
  Type* ty_=nullptr;

  // local variable offset
  int offset_=0;

  // Global Variable
  char* init_data=nullptr;
  // whether is a stirng
  bool is_string=false;

  // function parameter
  Object* params_=nullptr;
  // function body
  Node* body_=nullptr;
  // function variable list
  Object* loc_list_=nullptr;
  // function variable's stack size
  int stack_size_=0;
};

extern Object* locals;
extern Object* globals;

#endif   // OBJECT_GRUAD