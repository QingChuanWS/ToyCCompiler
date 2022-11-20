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
#include "tools.h"

#define ASM_GEN(...) AsmPrint(__VA_ARGS__)

// recursive over state.
template<typename T>
void AsmPrint(const T arg) {
  std::cout << arg << std::endl;
}

template<typename T, typename... Types>
void AsmPrint(const T first_arg, const Types... args) {
  std::cout << first_arg;
  AsmPrint(args...);
}

void CodeGenerator::GetVarAddr(Node* node) {
  if (node->kind_ == ND_VAR) {
    int offset = (node->name_ - 'a' + 1) * 8;
    ASM_GEN("  lea rax, [rbp-", offset, "]");
    ASM_GEN("  push rax");
    return;
  }

  Error("%c not an lvalue", node->name_);
}

void CodeGenerator::Load() {
  ASM_GEN("  pop rax");
  ASM_GEN("  mov rax, [rax]");
  ASM_GEN("  push rax");
}

void CodeGenerator::Store() {
  ASM_GEN("  pop rdi");
  ASM_GEN("  pop rax");
  ASM_GEN("  mov [rax], rdi");
  ASM_GEN("  push rdi");
}

void CodeGenerator::CodeGen(Node* node) {
  ASM_GEN(".intel_syntax noprefix");
  ASM_GEN(".global main");
  ASM_GEN("main:");

  // prologue; equally instruction "enter 0xD0,0".
  ASM_GEN("  push rbp");
  ASM_GEN("  mov rbp, rsp");
  ASM_GEN("  sub rsp, 0xD0");

  for (; node != nullptr; node = node->next) {
    AST_CodeGen(node);
  }

  // Epilogue; equally instruction leave.
  ASM_GEN("  .L.return:");
  ASM_GEN("  mov rsp, rbp");
  ASM_GEN("  pop rbp");

  ASM_GEN("  ret\n");
}

// post-order for code-gen
void CodeGenerator::AST_CodeGen(Node* node) {
  switch (node->kind_) {
  case ND_NUM: ASM_GEN("  push ", node->val_); return;
  case ND_EXPR_STMT:
    AST_CodeGen(node->lhs_);
    AsmPrint("  add rsp, 0x8");
    return;
  case ND_VAR: 
    GetVarAddr(node);
    Load();
    return;
  case ND_ASSIGN:
    GetVarAddr(node->lhs_);
    AST_CodeGen(node->rhs_);
    Store();
    return;
  case ND_RETURN:
    AST_CodeGen(node->lhs_);
    ASM_GEN("  pop rax");
    ASM_GEN("  jmp .L.return");
    return;
  default: break;
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
