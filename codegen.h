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

#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>

#include "node.h"
#include "utils.h"

class PrintFunctor {
 public:
  template <typename T>
  void operator()(T t) {
    if (use_std) {
      std::cout << t;
    } else {
      out << t;
    }
  }
  static PrintFunctor& GetInstance(const String& path = "-") {
    static PrintFunctor printor(path);
    return printor;
  }

 private:
  PrintFunctor(const String& path) {
    if (path.empty() || path == "-") {
      use_std = true;
    }
    out = std::ofstream(path);
    if (!out.is_open()) {
      Error("cannot open output file: %s.");
    }
    use_std = false;
  }
  ~PrintFunctor() {
    if (!use_std) {
      out.close();
    }
  }
  PrintFunctor(const PrintFunctor&) = delete;
  PrintFunctor operator=(const PrintFunctor&) = delete;
  std::ofstream out;
  bool use_std;
};

class CodeGenerator {
 public:
  // using specific output stream.
  CodeGenerator(const String& path) { PrintFunctor::GetInstance(path); }
  // don't allow copy constructor.
  CodeGenerator(const CodeGenerator&) = delete;
  // don't allow assign constructor.
  CodeGenerator& operator=(const CodeGenerator&) = delete;
  // deconstructor.
  ~CodeGenerator() {}
  // generator x86 assemly code.
  void CodeGen(ObjectPtr program);

 private:
  // emit data segment in assemly.
  void EmitData(ObjectPtr prog);
  // emit text segment in assemly.
  void EmitText(ObjectPtr prog);
  // get var node's address.
  void GetVarAddr(NodePtr& node);
  // push rax data to stask.
  void Push(void);
  // pop a stask element to specific register.
  void Pop(const char* arg);
  // emit exprssion assemly code.
  void ExprGen(NodePtr& node);
  // emit statement assemly code.
  void StmtGen(NodePtr& node);
  // load a data from memory address based on type size as ty.
  void Load(TypePtr ty);
  // store a data to memory address based on the type size as ty.
  void Store(TypePtr ty);
};

#endif  // !CODEGEN_GRUAD
