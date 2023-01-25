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

class Member {
 public:
  Member() = default;
  Member(TypePtr type, TokenPtr struct_name) : ty(type), name(struct_name) {}
  // get struct member offset.
  inline int GetOffset() { return offset; }

 public:
  // calculate struct align based on a list of Struct member.
  static int CalcuStructAlign(MemberPtr mem);
  // init struct member list offset, return the sum of offset.
  static int CalcuStructOffset(MemberPtr mem);

 private:
  friend Node;
  friend Type;
  friend Parser;

  // member type.
  TypePtr ty = nullptr;
  // member name.
  TokenPtr name = nullptr;
  // next member.
  MemberPtr next = nullptr;
  // member offset based on the struct head.
  int offset = 0;
};

#endif  // STRUCT_GRUAD