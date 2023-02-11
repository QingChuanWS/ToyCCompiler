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

static int depth = 0;

static const char* argreg8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static const char* argreg16[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
static const char* argreg32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static const char* argreg64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

#define ASM_GEN(...) Println<CodeGenPrinter>(__VA_ARGS__, "\n");

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
    case ND_COMMON:
      ExprGen(node->lhs);
      GetVarAddr(node->rhs);
      return;
    case ND_MUMBER:
      GetVarAddr(node->lhs);
      ASM_GEN("  add rax, ", node->mem->GetOffset());
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

void CodeGenerator::Load(TypePtr& ty) {
  if (ty->Is<TY_ARRAY>() || ty->Is<TY_STRUCT>() || ty->Is<TY_UNION>()) {
    // If it is a array, do not attempt to load a value to
    // the register because we can't load entire array to
    // register. As a result, the evaluation's result isn't
    // the array itself but the array address. This is where
    // "array is automatically converted to a pointer to the first
    // element of the array in C" occur.
    return;
  }
  if (ty->Size() == 1) {
    ASM_GEN("  movsbl eax, BYTE PTR [rax]");
  } else if (ty->Size() == 2) {
    ASM_GEN("  movswl eax, WORD PTR [rax]");

  } else if (ty->Size() == 4) {
    ASM_GEN("  movsxd rax, DWORD PTR [rax]");
  } else {
    ASM_GEN("  mov rax, [rax]");
  }
}

void CodeGenerator::Store(TypePtr& ty) {
  Pop("rdi");
  if (ty->Is<TY_STRUCT>() || ty->Is<TY_UNION>()) {
    for (int i = 0; i < ty->Size(); i++) {
      ASM_GEN("  mov r8b, [rax + ", i, "]");
      ASM_GEN("  mov [rdi + ", i, "], r8b");
    }
    return;
  }

  if (ty->Size() == 1) {
    ASM_GEN("  mov [rdi], al");
  } else if (ty->Size() == 2) {
    ASM_GEN("  mov [rdi], ax");
  } else if (ty->Size() == 4) {
    ASM_GEN("  mov [rdi], eax");
  } else {
    ASM_GEN("  mov [rdi], rax");
  }
}

void CodeGenerator::Cast(TypePtr from, TypePtr to) {
  static const char* i32i8 = "movsbl eax, al";
  static const char* i32i16 = "movswl eax, ax";
  static const char* i32i64 = "movsxd rax, eax";

  static Matrix<const char*> cast_table = {{nullptr, nullptr, nullptr, i32i64},
                                           {i32i8, nullptr, nullptr, i32i64},
                                           {i32i8, i32i16, nullptr, i32i64},
                                           {i32i8, i32i16, nullptr, nullptr}};

  auto GetTypeId = [](TypePtr& t) -> int {
    enum { I8 = 0, I16, I32, I64 };
    if (t->Is<TY_CHAR>()) {
      return I8;
    } else if (t->Is<TY_SHORT>()) {
      return I16;
    } else if (t->Is<TY_INT>()) {
      return I32;
    } else {
      return I64;
    }
  };
  auto cmp_zero = [](const TypePtr& ty) {
    if (ty->IsInteger() && ty->Size() <= 4) {
      ASM_GEN("  cmp eax, 0");
    } else {
      ASM_GEN("  cmp rax, 0");
    }
  };

  if (to->Is<TY_VOID>()) {
    return;
  }

  if (to->Is<TY_BOOL>()) {
    cmp_zero(from);
    ASM_GEN("  setne al");
    ASM_GEN("  movzx eax, al");
    return;
  }

  auto t1 = GetTypeId(from);
  auto t2 = GetTypeId(to);
  if (cast_table[t1][t2]) {
    ASM_GEN("  ", cast_table[t1][t2]);
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
    ASM_GEN("  .data");
    ASM_GEN("  .global ", var->obj_name);
    ASM_GEN(var->obj_name, ":");
    if (var->is_string) {
      for (int i = 0; i < var->ty->Size(); i++) {
        ASM_GEN("  .byte ", static_cast<int>(var->init_data[i]));
      }
    } else {
      ASM_GEN("  .zero ", var->ty->Size());
    }
  }
}

void CodeGenerator::StoreFunctionParameter(int reg, int offset, int sz) {
  switch (sz) {
    case 1:
      ASM_GEN("  mov [rbp - ", offset, "], ", argreg8[reg]);
      break;
    case 2:
      ASM_GEN("  mov [rbp - ", offset, "], ", argreg16[reg]);
      break;
    case 4:
      ASM_GEN("  mov [rbp - ", offset, "], ", argreg32[reg]);
      break;
    case 8:
      ASM_GEN("  mov [rbp - ", offset, "], ", argreg64[reg]);
      break;
    default:
      unreachable();
  }
}

void CodeGenerator::EmitText(ObjectPtr prog) {
  // using intel syntax
  // e.g. op dst, src
  ASM_GEN("  .intel_syntax noprefix");
  for (ObjectPtr fn = prog; fn != nullptr; fn = fn->next) {
    if (fn->IsGlobal()) {
      continue;
    }

    if (fn->is_static) {
      ASM_GEN("  .local ", fn->obj_name);
    } else {
      ASM_GEN("  .global ", fn->obj_name);
    }
    ASM_GEN(" .text");
    ASM_GEN(fn->obj_name, ":");
    cur_fn = fn;

    // prologue; equally instruction "enter 0xD0,0".
    ASM_GEN("  push rbp");
    ASM_GEN("  mov rbp, rsp");
    ASM_GEN("  sub rsp, ", fn->stack_size);

    int i = 0;
    for (ObjectPtr var = fn->params; var != nullptr; var = var->next) {
      StoreFunctionParameter(i++, var->offset, var->ty->Size());
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
  ASM_GEN("  .loc 1 ", node->name->GetLineNo());

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
  ASM_GEN("  .loc 1 ", node->name->GetLineNo());

  switch (node->kind) {
    case ND_NUM:
      ASM_GEN("  mov rax, ", node->val);
      return;
    case ND_NEG:
      ExprGen(node->lhs);
      ASM_GEN("  neg rax");
      return;
    case ND_VAR:
    case ND_MUMBER:
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
    case ND_COMMON:
      ExprGen(node->lhs);
      ExprGen(node->rhs);
      return;
    case ND_CAST:
      ExprGen(node->lhs);
      Cast(node->lhs->ty, node->ty);
      return;
    case ND_NOT:
      ExprGen(node->lhs);
      ASM_GEN("  cmp rax, 0");
      ASM_GEN("  sete al")
      ASM_GEN("  movzx rax, al")
      return ;
    case ND_BITNOT:
      ExprGen(node->lhs);
      ASM_GEN("   not rax")
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

  const char *ax, *di;
  if (node->lhs->ty->Is<TY_LONG>() || node->lhs->IsPointerNode()) {
    ax = "rax";
    di = "rdi";
  } else {
    ax = "eax";
    di = "edi";
  }

  switch (node->kind) {
    case ND_ADD:
      ASM_GEN("  add ", ax, ", ", di);
      return;
    case ND_SUB:
      ASM_GEN("  sub  ", ax, ", ", di);
      return;
    case ND_MUL:
      ASM_GEN("  imul  ", ax, ", ", di);
      return;
    case ND_DIV:
    case ND_MOD:
      if (node->lhs->ty->Size() == 8) {
        ASM_GEN("  cqo");
      } else {
        ASM_GEN("  cdq");
      }
      ASM_GEN("  idiv ", di);
      if(node->kind == ND_MOD){
        ASM_GEN("  mov rax, rdx")
      }
      return;
    case ND_EQ:
      ASM_GEN("  cmp ", ax, ", ", di);
      ASM_GEN("  sete al");
      ASM_GEN("  movzb rax, al");
      return;
    case ND_NE:
      ASM_GEN("  cmp ", ax, ", ", di);
      ASM_GEN("  setne al");
      ASM_GEN("  movzb rax, al");
      return;
    case ND_LT:
      ASM_GEN("  cmp ", ax, ", ", di);
      ASM_GEN("  setl al");
      ASM_GEN("  movzb rax, al");
      return;
    case ND_LE:
      ASM_GEN("  cmp ", ax, ", ", di);
      ASM_GEN("  setle al");
      ASM_GEN("  movzb rax, al");
      return;
    default:
      node->name->ErrorTok("invalid expression.");
  }
}
