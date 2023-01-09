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

#ifndef CODEGEN_GRUAD
#define CODEGEN_GRUAD

#include <iostream>

#include "node.h"

class CodeGenerator {
 public:
  void CodeGen(ObjectPtr prog);

 private:
  static void EmitData(ObjectPtr prog);
  static void EmitText(ObjectPtr prog);
  static void GetVarAddr(NodePtr& node);
  static void Push(void);
  static void Pop(const char* arg);
  static void ExprGen(NodePtr& node);
  static void StmtGen(NodePtr& node);
  static void Load(TypePtr ty);
  static void Store(TypePtr ty);

  static int depth;
};

#endif  // !CODEGEN_GRUAD
