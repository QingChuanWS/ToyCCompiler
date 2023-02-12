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

#include "type.h"

#include "node.h"
#include "struct.h"
#include "tools.h"
#include "utils.h"

TypePtr ty_long = std::make_shared<Type>(TY_LONG, 8, 8);
TypePtr ty_int = std::make_shared<Type>(TY_INT, 4, 4);
TypePtr ty_short = std::make_shared<Type>(TY_SHORT, 2, 2);
TypePtr ty_char = std::make_shared<Type>(TY_CHAR, 1, 1);
TypePtr ty_void = std::make_shared<Type>(TY_VOID, 1, 1);
TypePtr ty_bool = std::make_shared<Type>(TY_BOOL, 1, 1);

const Type& Type::Copy(const Type& ty) {
  *this = ty;
  // if (ty.base) {
  //   this->base = std::make_shared<Type>(*ty.base);
  // }
  // if (ty.mem) {
  //   this->mem = std::make_shared<Member>(*ty.mem);  /// copy array
  // }
  // if (ty.return_ty) {
  //   this->return_ty = std::make_shared<Type>(*ty.return_ty);
  // }
  // if (!ty.params.empty()) {
  //   // copy list.
  // }
  return *this;
}

bool Type::IsInteger() const {
  return kind == TY_BOOL || kind == TY_INT || kind == TY_CHAR || kind == TY_SHORT ||
         kind == TY_LONG || kind == TY_ENUM;
}

TypePtr Type::CreatePointerType(TypePtr base) {
  auto ty = std::make_shared<Type>(TY_PRT, 8, 8);
  ty->base = base;
  return ty;
}

TypePtr Type::CreateFunctionType(TypePtr ret_type, const TypeVector& params) {
  auto ty = std::make_shared<Type>(TY_FUNC, ret_type->size, 0);
  ty->return_ty = ret_type;
  ty->params = params;
  return ty;
}

TypePtr Type::CreateArrayType(TypePtr base, int array_len) {
  auto ty = std::make_shared<Type>(TY_ARRAY, base->size * array_len, base->align);
  ty->base = base;
  return ty;
}

TypePtr Type::CreateStructType(MemberVector mem) {
  auto ty = std::make_shared<Type>(TY_STRUCT, 1, 1);
  ty->align = Member::CalcuStructAlign(mem);
  ty->size = AlignTo(Member::CalcuStructOffset(mem), ty->align);
  ty->mem = mem;
  return ty;
}

TypePtr Type::CreateUnionType(MemberVector mem) {
  auto ty = std::make_shared<Type>(TY_UNION, 1, 1);
  for (auto m : mem) {
    if (ty->align < m->ty->align) {
      ty->align = m->ty->align;
    }
    if (ty->size < m->ty->size) {
      ty->size = m->ty->size;
    }
  }
  ty->size = AlignTo(ty->size, ty->align);
  ty->mem = mem;
  return ty;
}

const TokenPtr& Type::GetName() const {
  if (name == nullptr) {
    Error("The type has not name!");
  }
  return name;
}

// get struct member based on token.
MemberPtr Type::GetStructMember(TokenPtr tok) const {
  for (auto m : mem) {
    if (tok->Equal(m->name)) {
      return m;
    }
  }
  tok->ErrorTok("no such member.");
  return nullptr;
}

TypePtr Type::CreateEnumType() { return std::make_shared<Type>(TY_ENUM, 4, 4); }

TypePtr Type::GetCommonType(const TypePtr& ty1, const TypePtr& ty2) {
  if (ty1->IsPointer()) {
    return Type::CreatePointerType(ty1->base);
  }
  if (ty1->size == 8 || ty2->size == 8) {
    return ty_long;
  }
  return ty_int;
}

void Type::UsualArithConvert(NodePtr& lhs, NodePtr& rhs) {
  TypePtr ty = GetCommonType(lhs->ty, rhs->ty);
  lhs = Node::CreateCastNode(lhs->name, lhs, ty);
  rhs = Node::CreateCastNode(rhs->name, rhs, ty);
}

void Type::TypeInfer(NodePtr node) {
  if (node == nullptr || node->ty != nullptr) {
    return;
  }

  TypeInfer(node->lhs);
  TypeInfer(node->rhs);
  TypeInfer(node->cond);
  TypeInfer(node->then);
  TypeInfer(node->els);
  TypeInfer(node->init);
  TypeInfer(node->inc);

  for (NodePtr n = node->body; n != nullptr; n = n->next) {
    TypeInfer(n);
  }
  for (NodePtr n = node->args; n != nullptr; n = n->next) {
    TypeInfer(n);
  }

  switch (node->kind) {
    case ND_NUM:
      node->ty = ((node->val == (int)node->val) ? ty_int : ty_long);
      return;
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_MOD:
    case ND_BITAND:
    case ND_BITOR:
    case ND_BITXOR:
      UsualArithConvert(node->lhs, node->rhs);
      node->ty = node->lhs->ty;
      return;
    case ND_NEG: {
      TypePtr ty = GetCommonType(ty_int, node->lhs->ty);
      node->lhs = Node::CreateCastNode(node->lhs->name, node->lhs, ty);
      node->ty = ty;
      return;
    }
    case ND_ASSIGN:
      if (node->lhs->IsArrayNode()) {
        node->lhs->Error("not an lvalue");
      }
      if (!node->lhs->ty->Is<TY_STRUCT>() && !node->lhs->ty->Is<TY_UNION>()) {
        node->rhs = Node::CreateCastNode(node->rhs->name, node->rhs, node->lhs->ty);
      }
      node->ty = node->lhs->ty;
      return;
    case ND_EQ:
    case ND_NE:
    case ND_LE:
    case ND_LT:
      Type::UsualArithConvert(node->lhs, node->rhs);
      node->ty = ty_int;
      return;
    case ND_CALL:
      node->ty = ty_long;
      return;
    case ND_NOT:
    case ND_LOGAND:
    case ND_LOGOR:
      node->ty = ty_int;
      return;
    case ND_BITNOT:
      node->ty = node->lhs->ty;
      return;
    case ND_VAR:
      node->ty = node->var->GetType();
      return;
    case ND_COMMON:
      node->ty = node->rhs->ty;
      return;
    case ND_MUMBER:
      node->ty = node->mem->ty;
      return;
    case ND_ADDR:
      if (node->lhs->IsArrayNode()) {
        node->ty = CreatePointerType(node->lhs->ty->GetBase());
      } else {
        node->ty = CreatePointerType(node->lhs->ty);
      }
      return;
    case ND_DEREF:
      if (!node->lhs->IsPointerNode()) {
        node->Error("invalid pointer reference!");
      }
      if (node->lhs->ty->GetBase()->Is<TY_VOID>()) {
        node->Error("dereferencing a void pointer.");
      }
      node->ty = node->lhs->ty->GetBase();
      return;
    case ND_STMT_EXPR:
      if (node->body != nullptr) {
        NodePtr stmt = node->body;
        while (stmt->next != nullptr) {
          stmt = stmt->next;
        }
        if (stmt->kind == ND_EXPR_STMT) {
          node->ty = stmt->lhs->ty;
          return;
        }
      }
      node->Error("statement expression return void is not supported.");
    default:
      return;
  }
}