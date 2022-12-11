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
#ifndef FUNCTION_GRUAD
#define FUNCTION_GRUAD

#include "node.h"
#include "token.h"
#include "type.h"
#include "var.h"

struct Function {
  Function()
      : next_(nullptr)
      , body_(nullptr)
      , name_(nullptr)
      , var_list_(nullptr)
      , stack_size_(0) {}

  Function(Node* body, Var* var_list = nullptr)
      : next_(nullptr)
      , body_(body)
      , name_(nullptr)
      , var_list_(var_list)
      , stack_size_(0) {}

  Function(Token** rest, Token* tok);

  // parsing token list and generate AST.
  static Function* Parse(Token* tok);

  static void FunctionFree(Function* head);
  static void OffsetCal(Function* prog);

  Function* next_;

  Node* body_;
  char* name_;
  Var*  var_list_;

  int stack_size_;
};


#endif   // !FUNCTION_GRUAD
