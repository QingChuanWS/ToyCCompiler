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

// Code generator pinter.
class CodeGenPrinter {
 public:
  static CodeGenPrinter& GetInstance(const Config& config = Config()) {
    static CodeGenPrinter printor(config);
    return printor;
  }

  template <typename T>
  static void Print(T t) {
    bool use_std = CodeGenPrinter::GetInstance().use_std;
    if (use_std) {
      std::cout << t;
    } else {
      CodeGenPrinter::GetInstance().out << t;
    }
  }

 private:
  CodeGenPrinter(const Config& config) {
    const String& input = config.input_path;
    const String& output = config.output_path;
    if (output.empty() || output == "-") {
      use_std = true;
      std::cout << ".file 1 \"" << input << "\"\n";
    }
    out = std::ofstream(output);
    if (!out.is_open()) {
      Error("cannot open output file: %s.");
    }
    use_std = false;
    out << ".file 1 \"" << input << "\"\n";
  }

  ~CodeGenPrinter() {
    if (!use_std) {
      out.close();
    }
  }
  CodeGenPrinter(const CodeGenPrinter&) = delete;
  CodeGenPrinter operator=(const CodeGenPrinter&) = delete;
  std::ofstream out;
  bool use_std;
};

// code generator.
class CodeGenerator {
 public:
  // using specific output stream.
  CodeGenerator(const Config& config) { CodeGenPrinter::GetInstance(config); }
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
  void Load(TypePtr ty);
  // store a data to memory address based on the type size as ty.
  void Store(TypePtr ty);
  // store passed-by-register arguments to the stack.
  void StoreFunctionParameter(int reg, int offset, int sz);
};

#endif  // !CODEGEN_GRUAD
