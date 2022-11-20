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
#include "codegen.h"
#include "node.h"

#define ASM_GEN(...) ASMGenerator(__VA_ARGS__)

// recursive over state.
template<typename T>
void ASMGenerator(const T arg) {
  std::cout << arg << std::endl;
}

template<typename T, typename... Types>
void ASMGenerator(const T first_arg, const Types... args) {
  std::cout << first_arg;
  ASMGenerator(args...);
}

void CodeGenerator::CodeGen(Node* node) {
  ASM_GEN(".intel_syntax noprefix");
  ASM_GEN(".global main");
  ASM_GEN("main:");

  for (; node != nullptr; node = node->next) {
    AST_CodeGen(node);

    ASM_GEN("  pop rax\n");
  }

  ASM_GEN("  ret\n");
}

// post-order for code-gen
void CodeGenerator::AST_CodeGen(Node* node) {
  switch (node->kind_) {
  case ND_NUM:
    ASM_GEN("  push ", node->val_);
    return;
  case ND_RETURN:
    AST_CodeGen(node->lhs_);
    ASM_GEN("  pop rax");
    ASM_GEN("  ret");
    return;
  default:
    break;
  }

  AST_CodeGen(node->lhs_);
  AST_CodeGen(node->rhs_);

  ASM_GEN("  pop rdi");
  ASM_GEN("  pop rax");

  switch (node->kind_) {
  case ND_ADD: ASM_GEN("  add rax, rdi"); break;
  case ND_SUB: ASM_GEN("  sub rax, rdi"); break;
  case ND_MUL: ASM_GEN("  imul rax, rdi"); break;
  case ND_DIV:
    ASM_GEN("  cqo");
    ASM_GEN("  idiv rdi");
    break;
  case ND_EQ:
    ASM_GEN("  cmp rax, rdi");
    ASM_GEN("  sete al");
    ASM_GEN("  movzb rax, al");
    break;
  case ND_NE:
    ASM_GEN("  cmp rax, rdi");
    ASM_GEN("  setne al");
    ASM_GEN("  movzb rax, al");
    break;
  case ND_LT:
    ASM_GEN("  cmp rax, rdi");
    ASM_GEN("  setl al");
    ASM_GEN("  movzb rax, al");
    break;
  case ND_LE:
    ASM_GEN("  cmp rax, rdi");
    ASM_GEN("  setle al");
    ASM_GEN("  movzb rax, al");
    break;
  default: Error("error node type !"); break;
  }

  ASM_GEN("  push rax");
}
