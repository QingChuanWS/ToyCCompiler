/*
 * This project is exclusively owned by QingChuanWS and shall not be used for
 * commercial and profitting purpose without QingChuanWS's permission.
 *
 * @Author: bingshan45@163.com
 * Github: https://github.com/QingChuanWS
 * @Description:
 *
 * Copyright (c) 2022 by QingChuanWS, All Rights Reserved.
 */
#ifndef OBJECT_GRUAD
#define OBJECT_GRUAD

#include "token.h"
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

  Object(Objectkind kind, Type* ty, Object** next);

  // create a function based on token list.
  static Token* CreateFunction(Token* tok, Type* basety);
  // create global variables based on token list.
  static Token* CreateGlobal(Token* tok, Type* basety);
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
  // check whether is a global variable or function
  // based on a token.
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
  // label the object type
  Objectkind kind_;
  // for object list
  Object* next_;
  // variable name
  char* name_;
  // Type
  Type* ty_;

  // local variable offset
  int offset_;

  // function parameter
  Object* params_;
  // function body
  Node* body_;
  // function variable list
  Object* loc_list_;
  // function variable's stack size
  int stack_size_;
};

extern Object* locals;
extern Object* globals;

#endif   // OBJECT_GRUAD