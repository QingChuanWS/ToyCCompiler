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

#include <cstddef>
#include <iostream>

#include "node.h"
#include "object.h"
#include "tools.h"
#include "type.h"
#include "utils.h"

static int Count() {
  static int count = 0;
  return count++;
}

int depth = 0;
ObjectPtr cur_fn = nullptr;

const char* argreg8[6] = {"dil", "sil", "cl", "dl", "r8b", "r9b"};
const char* argreg64[6] = {"rdi", "rsi", "rcx", "rdx", "r8", "r9"};

#define ASM_GEN(...) Println<PrintFunctor>(__VA_ARGS__, "\n");

void CodeGenerator::GetVarAddr(NodePtr& node) {
  switch (node->kind) {
    case ND_VAR:
      if (node->var->IsLocal()) {
        ASM_GEN("  lea rax, [rbp - ", node->var->offset, "]");
      } else {
        ASM_GEN("  lea rax, [rip + ", node->var->obj_name, "]");
      }
      return;
    case ND_DEREF:
      ExprGen(node->lhs);
      return;
    default:
      break;
  }

  node->name->ErrorTok("not an lvalue");
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
  if (ty->kind == TY_ARRAY) {
    // If it is a array, do not attempt to load a value to
    // the register because we can't load entire array to
    // register. As a result, the evaluation's result isn't
    // the array itself but the array address. This is where
    // "array is automatically converted to a pointer to the first
    // element of the array in C" occur.
    return;
  }
  if (ty->size == 1) {
    ASM_GEN("  mov al, [rax]");
  } else {
    ASM_GEN("  mov rax, [rax]");
  }
}

void CodeGenerator::Store(TypePtr ty) {
  Pop("rdi");
  if (ty->size == 1) {
    ASM_GEN("  mov [rdi], al");
  } else {
    ASM_GEN("  mov [rdi], rax");
  }
}

void CodeGenerator::CodeGen(ObjectPtr program) {
  program->OffsetCal();
  EmitData(program);
  EmitText(program);
}

void CodeGenerator::EmitData(ObjectPtr prog) {
  for (ObjectPtr var = prog; var != nullptr; var = var->next) {
    if (var->IsFunction()) {
      continue;
    }
    ASM_GEN("  .data", "\n");
    ASM_GEN("  .global ", var->obj_name, "\n");
    ASM_GEN(var->obj_name, ":");
    if (var->is_string) {
      for (int i = 0; i < var->ty->size; i++) {
        ASM_GEN("  .byte ", static_cast<int>(var->init_data[i]), "\n");
      }
    } else {
      ASM_GEN("  .zero ", var->ty->size);
    }
  }
}

void CodeGenerator::EmitText(ObjectPtr prog) {
  // using intel syntax
  // e.g. op dst, src
  ASM_GEN(".intel_syntax noprefix");
  for (ObjectPtr fn = prog; fn != nullptr; fn = fn->next) {
    if (fn->IsGlobal()) {
      continue;
    }

    ASM_GEN("  .global ", fn->obj_name);
    ASM_GEN(" .text");
    ASM_GEN(fn->obj_name, ":");
    cur_fn = fn;

    // prologue; equally instruction "enter 0xD0,0".
    ASM_GEN("  push rbp");
    ASM_GEN("  mov rbp, rsp");
    ASM_GEN("  sub rsp, ", fn->stack_size);

    int i = 0;
    for (ObjectPtr var = fn->params; var != nullptr; var = var->next) {
      if (var->ty->size == 1) {
        ASM_GEN("  mov [rbp - ", var->offset, "], ", argreg8[i++]);
      } else {
        ASM_GEN("  mov [rbp - ", var->offset, "], ", argreg64[i++]);
      }
    }

    // Emit code
    StmtGen(fn->body);
    DEBUG(depth == 0);

    // Epilogue; equally instruction leave.
    ASM_GEN(".L.return.", fn->obj_name, ":");
    ASM_GEN("  mov rsp, rbp");
    ASM_GEN("  pop rbp");
    ASM_GEN("  ret");
  }
}

void CodeGenerator::StmtGen(NodePtr& node) {
  switch (node->kind) {
    case ND_EXPR_STMT:
      ExprGen(node->lhs);
      return;
    case ND_BLOCK:
      for (NodePtr& n = node->body; n != nullptr; n = n->next) {
        StmtGen(n);
      }
      return;
    case ND_RETURN:
      ExprGen(node->lhs);
      ASM_GEN("  jmp .L.return.", cur_fn->obj_name);
      return;
    case ND_IF: {
      int seq = Count();
      ExprGen(node->cond);
      ASM_GEN("  cmp rax, 0");
      ASM_GEN("  je .L.else.", seq);
      StmtGen(node->then);
      ASM_GEN("  jmp .L.end.", seq);
      ASM_GEN(".L.else.", seq, ":");
      if (node->els != nullptr) {
        StmtGen(node->els);
      }
      ASM_GEN(".L.end.", seq, ":");
      return;
    }
    case ND_FOR: {
      int seq = Count();
      if (node->init != nullptr) StmtGen(node->init);
      ASM_GEN(".L.begin.", seq, ":");
      if (node->cond != nullptr) {
        ExprGen(node->cond);
        ASM_GEN("  cmp rax, 0");
        ASM_GEN("  je .L.end.", seq);
      }
      StmtGen(node->then);
      if (node->inc != nullptr) {
        ExprGen(node->inc);
      }
      ASM_GEN("  jmp .L.begin.", seq);
      ASM_GEN(".L.end.", seq, ":");
      return;
    }
    default:
      node->name->ErrorTok("invalid statement");
  }
}

// post-order for code-gen
void CodeGenerator::ExprGen(NodePtr& node) {
  switch (node->kind) {
    case ND_NUM:
      ASM_GEN("  mov rax, ", node->val);
      return;
    case ND_NEG:
      ExprGen(node->lhs);
      ASM_GEN("  neg rax");
      return;
    case ND_VAR:
      GetVarAddr(node);
      Load(node->ty);
      return;
    case ND_DEREF:
      ExprGen(node->lhs);
      Load(node->ty);
      return;
    case ND_ADDR:
      GetVarAddr(node->lhs);
      return;
    case ND_ASSIGN:
      GetVarAddr(node->lhs);
      Push();
      ExprGen(node->rhs);
      Store(node->ty);
      return;
    case ND_STMT_EXPR:
      for (NodePtr n = node->body; n != nullptr; n = n->next) {
        StmtGen(n);
      }
      return;
    case ND_CALL: {
      int nargs = 0;
      for (NodePtr& arg = node->args; arg != nullptr; arg = arg->next) {
        ExprGen(arg);
        Push();
        nargs++;
      }
      for (int i = nargs - 1; i >= 0; i--) {
        Pop(argreg64[i]);
      }

      ASM_GEN("  mov rax, 0");
      ASM_GEN("  call ", node->call);
      return;
    }
    default:
      break;
  }

  ExprGen(node->rhs);
  Push();
  ExprGen(node->lhs);
  Pop("rdi");

  switch (node->kind) {
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
      node->name->ErrorTok("invalid expression.");
  }
}
