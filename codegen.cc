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
#include "object.h"
#include "tools.h"
#include "type.h"

static int Count() {
  static int count = 0;
  return count++;
}

int CodeGenerator::depth = 0;

const char* argreg8[6] = {"dil", "sil", "cl", "dl", "r8b", "r9b"};
const char* argreg64[6] = {"rdi", "rsi", "rcx", "rdx", "r8", "r9"};

 ObjectPtr cur_fn = nullptr;

#define ASM_GEN(...) AsmPrint(__VA_ARGS__)

// recursive over state.
template <typename T>
static void AsmPrint(const T arg) {
  std::cout << arg << std::endl;
}

template <typename T, typename... Types>
static void AsmPrint(const T first_arg, const Types... args) {
  std::cout << first_arg;
  AsmPrint(args...);
}

void CodeGenerator::GetVarAddr(NodePtr& node) {
  switch (node->kind_) {
    case ND_VAR:
      if (node->var_->IsLocal()) {
        ASM_GEN("  lea rax, [rbp - ", node->var_->offset_, "]");
      } else {
        ASM_GEN("  lea rax, [rip + ", node->var_->name_, "]");
      }
      return;
    case ND_DEREF:
      ExprGen(node->lhs_);
      return;
    default:
      break;
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

void CodeGenerator::Load(TypePtr ty) {
  if (ty->kind_ == TY_ARRAY) {
    // If it is a array, do not attempt to load a value to
    // the register because we can't load entire array to
    // register. As a result, the evaluation's result isn't
    // the array itself but the array address. This is where
    // "array is automatically converted to a pointer to the first
    // element of the array in C" occur.
    return;
  }
  if (ty->size_ == 1) {
    ASM_GEN("  mov al, [rax]");
  } else {
    ASM_GEN("  mov rax, [rax]");
  }
}

void CodeGenerator::Store(TypePtr ty) {
  Pop("rdi");
  if (ty->size_ == 1) {
    ASM_GEN("  mov [rdi], al");
  } else {
    ASM_GEN("  mov [rdi], rax");
  }
}

void CodeGenerator::CodeGen( ObjectPtr prog) {
  prog->OffsetCal();
  EmitData(prog);
  EmitText(prog);
}

void CodeGenerator::EmitData( ObjectPtr prog) {
  for ( ObjectPtr var = prog; var != nullptr; var = var->next_) {
    if (var->IsFunction()) {
      continue;
    }
    ASM_GEN("  .data");
    ASM_GEN("  .global ", var->name_);
    ASM_GEN(var->name_, ":");
    if (var->is_string) {
      for (int i = 0; i < var->ty_->size_; i++) {
        ASM_GEN("  .byte ", static_cast<int>(var->init_data[i]));
      }
    } else {
      ASM_GEN("  .zero ", var->ty_->size_);
    }
  }
}

void CodeGenerator::EmitText( ObjectPtr prog) {
  // using intel syntax
  // e.g. op dst, src
  ASM_GEN(".intel_syntax noprefix");
  for ( ObjectPtr fn = prog; fn != nullptr; fn = fn->next_) {
    if (fn->IsGlobal()) {
      continue;
    }

    ASM_GEN("  .global ", fn->name_);
    ASM_GEN(" .text");
    ASM_GEN(fn->name_, ":");
    cur_fn = fn;

    // prologue; equally instruction "enter 0xD0,0".
    ASM_GEN("  push rbp");
    ASM_GEN("  mov rbp, rsp");
    ASM_GEN("  sub rsp, ", fn->stack_size_);

    int i = 0;
    for ( ObjectPtr var = fn->params_; var != nullptr; var = var->next_) {
      if (var->ty_->size_ == 1) {
        ASM_GEN("  mov [rbp - ", var->offset_, "], ", argreg8[i++]);
      } else {
        ASM_GEN("  mov [rbp - ", var->offset_, "], ", argreg64[i++]);
      }
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

void CodeGenerator::StmtGen(NodePtr& node) {
  switch (node->kind_) {
    case ND_EXPR_STMT:
      ExprGen(node->lhs_);
      return;
    case ND_BLOCK:
      for (NodePtr& n = node->body_; n != nullptr; n = n->next_) {
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
      if (node->init_ != nullptr) StmtGen(node->init_);
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
    default:
      node->tok_->ErrorTok("invalid statement");
  }
}

// post-order for code-gen
void CodeGenerator::ExprGen(NodePtr& node) {
  switch (node->kind_) {
    case ND_NUM:
      ASM_GEN("  mov rax, ", node->val_);
      return;
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
    case ND_ADDR:
      GetVarAddr(node->lhs_);
      return;
    case ND_ASSIGN:
      GetVarAddr(node->lhs_);
      Push();
      ExprGen(node->rhs_);
      Store(node->ty_);
      return;
    case ND_CALL: {
      int nargs = 0;
      for (NodePtr& arg = node->args_; arg != nullptr; arg = arg->next_) {
        ExprGen(arg);
        Push();
        nargs++;
      }

      for (int i = nargs - 1; i >= 0; i--) {
        Pop(argreg64[i]);
      }

      ASM_GEN("  mov rax, 0");
      ASM_GEN("  call ", node->call_);
      return;
    }
    default:
      break;
  }

  ExprGen(node->rhs_);
  Push();
  ExprGen(node->lhs_);
  Pop("rdi");

  switch (node->kind_) {
    case ND_ADD:
      ASM_GEN("  add rax, rdi");
      return;
    case ND_SUB:
      ASM_GEN("  sub rax, rdi");
      return;
    case ND_MUL:
      ASM_GEN("  imul rax, rdi");
      return;
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
    default:
      node->tok_->ErrorTok("invalid expression.");
  }
}
