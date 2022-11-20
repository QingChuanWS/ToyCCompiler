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

#ifndef CODEGEN_GRUAD
#define CODEGEN_GRUAD

#include "node.h"

#include <iostream>

class CodeGenerator {
 public:
  void CodeGen(Node* node);

 private:
  static void GetVarAddr(Node* node);
  static void Load();
  static void Store();
  static void AST_CodeGen(Node* node);
};

#endif   // !CODEGEN_GRUAD
