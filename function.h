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
#include "var.h"

struct Function {
  Function(Node* node, Var* var_head = nullptr)
      : node_(node)
      , var_head_(var_head)
      , stack_size_(0) {}

  // parsing token list and generate AST.
  static Function Parse(Token* tok);

  void FunctionFree();
  void OffsetCal();

  Node* node_;
  Var*  var_head_;
  int   stack_size_;
};


#endif   // !FUNCTION_GRUAD
