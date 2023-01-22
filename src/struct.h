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
#ifndef STRUCT_GRUAD
#define STRUCT_GRUAD

#include "parser.h"
#include "utils.h"

class Struct {
 public:
  Struct() = default;
  Struct(TypePtr type, TokenPtr struct_name) : ty(type), name(struct_name) {}
  int GetOffset() { return offset; }
  // calculate struct align based on a list of Struct member.
  static int CalcuAlign(StructPtr mem);
  // init struct member list offset, return the sum of offset.
  static int CalcuOffset(StructPtr mem);

 private:
  friend Node;
  friend Type;
  friend Parser;

  TypePtr ty = nullptr;
  TokenPtr name = nullptr;
  StructPtr next = nullptr;
  int offset = 0;
};

#endif  // STRUCT_GRUAD