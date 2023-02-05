/*
 * This project is exclusively owned by QingChuanWS and shall not be used for
 * commercial and profitting purpose without QingChuanWS's permission.
 *
 * @ Author: bingshan45@163.com
 * @ Github: https://github.com/QingChuanWS
 * @ Description:
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

// Code generator pinter.
class CodeGenPrinter {
 public:
  static CodeGenPrinter& GetInstance(const Config& cfg = Config()) {
    static CodeGenPrinter printor(cfg);
    return printor;
  }

  template <typename T>
  static void Print(T t) {
    if (CodeGenPrinter::GetInstance().use_std) {
      std::cout << t;
    } else {
      *CodeGenPrinter::GetInstance().out << t;
    }
  }

 private:
  explicit CodeGenPrinter(const Config& cfg) {
    const String& input = cfg.input_path;
    const String& output = cfg.output_path;
    if (output.empty() || output == "-") {
      std::cout << ".file 1 \"" << input << "\"\n";
      use_std = true;
    }
    out = std::unique_ptr<std::ofstream>(new std::ofstream(output));
    if (!out->is_open()) {
      Error("cannot open output file: %s.");
    }
    *out << ".file 1 \"" << input << "\"\n";
  }

  CodeGenPrinter(const CodeGenPrinter&) = delete;
  CodeGenPrinter operator=(const CodeGenPrinter&) = delete;
  std::unique_ptr<std::ofstream> out;
  bool use_std = false;
};

// code generator.
class CodeGenerator {
 public:
  // using specific output stream.
  explicit CodeGenerator(const Config& cfg) { CodeGenPrinter::GetInstance(cfg); }
  // don't allow copy constructor.
  CodeGenerator(const CodeGenerator&) = delete;
  // don't allow assign constructor.
  CodeGenerator& operator=(const CodeGenerator&) = delete;
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
  void Load(TypePtr& ty);
  // store a data to memory address based on the type size as ty.
  void Store(TypePtr& ty);
  // store passed-by-register arguments to the stack.
  void StoreFunctionParameter(int reg, int offset, int sz);
  // cast type
  void Cast(TypePtr from, TypePtr to);
};

#endif  // !CODEGEN_GRUAD
