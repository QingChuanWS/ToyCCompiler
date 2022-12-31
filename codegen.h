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
  void CodeGen(Object* prog);

 private:
  static void EmitData(Object* prog);
  static void EmitText(Object* prog);
  static void GetVarAddr(Node* node);
  static void Push(void);
  static void Pop(const char* arg);
  static void ExprGen(Node* node);
  static void StmtGen(Node* node);
  static void Load(Type* ty);
  static void Store();

  static int         depth;
  static const char* argreg[6];
};

#endif   // !CODEGEN_GRUAD
