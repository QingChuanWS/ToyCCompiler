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

#include "function.h"
#include "node.h"
#include "tools.h"
#include "type.h"

static int Count() {
  static int count = 0;
  return count++;
}

int CodeGenerator::depth = 0;

const char* CodeGenerator::argreg[6] = {"rdi", "rsi", "rcx", "rdx", "r8", "r9"};

Function* CodeGenerator::cur_fn = nullptr;

#define ASM_GEN(...) AsmPrint(__VA_ARGS__)

// recursive over state.
template<typename T>
static void AsmPrint(const T arg) {
  std::cout << arg << std::endl;
}

template<typename T, typename... Types>
static void AsmPrint(const T first_arg, const Types... args) {
  std::cout << first_arg;
  AsmPrint(args...);
}

void CodeGenerator::GetVarAddr(Node* node) {
  switch (node->kind_) {
  case ND_VAR: ASM_GEN("  lea rax, [rbp - ", node->var_->offset_, "]"); return;
  case ND_DEREF: ExprGen(node->lhs_); return;
  default: break;
  }

  node->tok_->ErrorTok("not an lvalue");
}

void CodeGenerator::Push() {
  ASM_GEN("  push rax");
  depth++;
}

void CodeGenerator::Pop(const char* arg) {
  ASM_GEN("  pop ", arg);
  depth--;
}

void CodeGenerator::Load(Type* ty) {
  if (ty->kind_ == TY_ARRAY) {
    // If it is a array, do not attempt to load a value to
    // the register because we can't load entire array to
    // register. As a result, the evaluation's result isn't
    // the array itself but the array address. This is where
    // "array is automatically converted to a pointer to the first
    // element of the array in C" occur.
    return;
  }
  ASM_GEN("  mov rax, [rax]");
}

void CodeGenerator::Store() {
  Pop("rdi");
  ASM_GEN("  mov [rdi], rax");
}

void CodeGenerator::CodeGen(Function* prog) {
  prog->OffsetCal();

  // using intel syntax
  // e.g. op dst, src
  ASM_GEN(".intel_syntax noprefix");
  for (Function* fn = prog; fn != nullptr; fn = fn->next_) {
    ASM_GEN("  .global ", fn->name_);
    ASM_GEN(fn->name_, ":");
    cur_fn = fn;

    // prologue; equally instruction "enter 0xD0,0".
    ASM_GEN("  push rbp");
    ASM_GEN("  mov rbp, rsp");
    ASM_GEN("  sub rsp, ", fn->stack_size_);

    int i = 0;
    for (Var* var = fn->params; var != nullptr; var = var->next_) {
      ASM_GEN("  mov [rbp - ", var->offset_, "], ", argreg[i++]);
    }

    // Emit code
    StmtGen(fn->body_);
    DEBUG(depth == 0);

    // Epilogue; equally instruction leave.
    ASM_GEN(".L.return.", fn->name_, ":");
    ASM_GEN("  mov rsp, rbp");
    ASM_GEN("  pop rbp");
    ASM_GEN("  ret");
  }
}

void CodeGenerator::StmtGen(Node* node) {
  switch (node->kind_) {
  case ND_EXPR_STMT: ExprGen(node->lhs_); return;
  case ND_BLOCK:
    for (Node* n = node->body_; n != nullptr; n = n->next_) {
      StmtGen(n);
    }
    return;
  case ND_RETURN:
    ExprGen(node->lhs_);
    ASM_GEN("  jmp .L.return.", cur_fn->name_);
    return;
  case ND_IF: {
    int seq = Count();
    ExprGen(node->cond_);
    ASM_GEN("  cmp rax, 0");
    ASM_GEN("  je .L.else.", seq);
    StmtGen(node->then_);
    ASM_GEN("  jmp .L.end.", seq);
    ASM_GEN(".L.else.", seq, ":");
    if (node->els_ != nullptr) {
      StmtGen(node->els_);
    }
    ASM_GEN(".L.end.", seq, ":");
    return;
  }
  case ND_FOR: {
    int seq = Count();
    if (node->init_ != nullptr)
      StmtGen(node->init_);
    ASM_GEN(".L.begin.", seq, ":");
    if (node->cond_ != nullptr) {
      ExprGen(node->cond_);
      ASM_GEN("  cmp rax, 0");
      ASM_GEN("  je .L.end.", seq);
    }
    StmtGen(node->then_);
    if (node->inc_ != nullptr) {
      ExprGen(node->inc_);
    }
    ASM_GEN("  jmp .L.begin.", seq);
    ASM_GEN(".L.end.", seq, ":");
    return;
  }
  default: node->tok_->ErrorTok("invalid statement");
  }
}

// post-order for code-gen
void CodeGenerator::ExprGen(Node* node) {
  switch (node->kind_) {
  case ND_NUM: ASM_GEN("  mov rax, ", node->val_); return;
  case ND_NEG:
    ExprGen(node->lhs_);
    ASM_GEN("  neg rax");
    return;
  case ND_VAR:
    GetVarAddr(node);
    Load(node->ty_);
    return;
  case ND_DEREF:
    ExprGen(node->lhs_);
    Load(node->ty_);
    return;
  case ND_ADDR: GetVarAddr(node->lhs_); return;
  case ND_ASSIGN:
    GetVarAddr(node->lhs_);
    Push();
    ExprGen(node->rhs_);
    Store();
    return;
  case ND_CALL: {
    int nargs = 0;
    for (Node* arg = node->args_; arg != nullptr; arg = arg->next_) {
      ExprGen(arg);
      Push();
      nargs++;
    }

    for (int i = nargs - 1; i >= 0; i--) {
      Pop(argreg[i]);
    }

    ASM_GEN("  mov rax, 0");
    ASM_GEN("  call ", node->call_);
    return;
  }
  default: break;
  }

  ExprGen(node->rhs_);
  Push();
  ExprGen(node->lhs_);
  Pop("rdi");

  switch (node->kind_) {
  case ND_ADD: ASM_GEN("  add rax, rdi"); return;
  case ND_SUB: ASM_GEN("  sub rax, rdi"); return;
  case ND_MUL: ASM_GEN("  imul rax, rdi"); return;
  case ND_DIV:
    ASM_GEN("  cqo");
    ASM_GEN("  idiv rdi");
    return;
  case ND_EQ:
    ASM_GEN("  cmp rax, rdi");
    ASM_GEN("  sete al");
    ASM_GEN("  movzb rax, al");
    return;
  case ND_NE:
    ASM_GEN("  cmp rax, rdi");
    ASM_GEN("  setne al");
    ASM_GEN("  movzb rax, al");
    return;
  case ND_LT:
    ASM_GEN("  cmp rax, rdi");
    ASM_GEN("  setl al");
    ASM_GEN("  movzb rax, al");
    return;
  case ND_LE:
    ASM_GEN("  cmp rax, rdi");
    ASM_GEN("  setle al");
    ASM_GEN("  movzb rax, al");
    return;
  default: node->tok_->ErrorTok("invalid expression.");
  }
}
