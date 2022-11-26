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
#include "function.h"

Function Function::Parse(Token* tok) {
  locals = nullptr;

  Node* cur = Node::Program(tok);
  return Function(cur, locals);
}

static int Align2(int n, int align) {
  return (n + align - 1) / align * align;
}

void Function::OffsetCal() {
  int offset = 0;
  for (Var* cur = var_head_; cur != nullptr; cur = cur->next_) {
    offset += 8;
    cur->offset_ = offset;
  }
  stack_size_ = Align2(offset, 16);;
}

void Function::FunctionFree() {
  Node::NodeListFree(this->node_);
  Var::VarFree(this->var_head_);
}