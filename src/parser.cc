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

#include "parser.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "node.h"
#include "object.h"
#include "scope.h"
#include "token.h"
#include "tools.h"
#include "type.h"
#include "utils.h"

// Lookahead tokens and returns true if a given token is a start
// of a function definition or declaration.
static bool IsFuncToks(TokenPtr tok) {
  if (tok->Equal(";")) {
    return false;
  }
  while (tok->Equal("*")) {
    tok = Token::GetNext<1>(tok);
  }
  if (!tok->Is<TK_IDENT>()) {
    tok->ErrorTok("expected a variable name.");
  }
  tok = Token::GetNext<1>(tok);
  if (tok->Equal("(")) {
    return true;
  }
  return false;
}

ASTree Parser::Run(TokenPtr tok) {
  ASTree ast;
  // enter scope
  Scope::EnterScope(scope);
  while (!tok->Is<TK_EOF>()) {
    auto attr = std::make_shared<VarAttr>();
    TypePtr basety = Parser::Declspec(&tok, tok, attr, ast);
    // parse typedef statement.
    if (attr->is_typedef) {
      TypePtrVector ty_list = Parser::TypedefDecl(&tok, tok, basety, ast);
      for (auto t : ty_list) {
        Scope::PushVarScope(t->name->GetIdent())->tydef = t;
      }
      continue;
    }
    // parse function.
    if (IsFuncToks(tok)) {
      tok = Parser::GlobalFunction(tok, basety, attr, ast);
      continue;
    }
    // parse global variable.
    TypePtrVector gtype_vec = GlobalVar(&tok, tok, basety, ast);
    for (auto gty : gtype_vec) {
      Object::CreateGlobalVar(gty->name->GetIdent(), gty, ast.globals);
    }
  }
  // leave scope.
  Scope::LevarScope(scope);
  return ast;
}

TokenPtr Parser::GlobalFunction(TokenPtr tok, TypePtr basety, VarAttrPtr attr, ASTree& ct) {
  TypePtr ty = Parser::Declarator(&tok, tok, basety, ct);
  const String& name = ty->name->GetIdent();
  ct.cur_fn = Object::CreateVar(Objectkind::OB_FUNCTION, name, ty);

  ct.locals.clear();
  FuncAttr func_attr = {0, false, false};
  // function declaration
  if (tok->Equal(";")) {
    func_attr.is_defination = true;
    tok = tok->SkipToken(";");
    return tok;
  }
  func_attr.is_static = attr->is_static;

  // create scope.
  Scope::EnterScope(scope);

  // funtion defination.
  for (auto i = ty->params.rbegin(); i != ty->params.rend(); ++i) {
    ObjectPtr v = Object::CreateLocalVar((*i)->name->GetIdent(), *i, ct.locals);
  }

  ObjectList params = ct.locals;
  NodePtr body = Parser::Program(&tok, tok, ct);
  ObjectList loc_list = ct.locals;

  // leave scope.
  Scope::LevarScope(scope);

  ct.globals.push_back(
      Object::CreateFunction(name, ty, std::move(params), std::move(loc_list), body, func_attr));

  Node::UpdateGotoLabel();
  return tok;
}

TypePtrVector Parser::GlobalVar(TokenPtr* rest, TokenPtr tok, TypePtr basety, ASTree& ct) {
  bool first = true;

  TypePtrVector res;
  while (!tok->Equal(";")) {
    if (!first) {
      tok = tok->SkipToken(",");
    }
    first = false;
    TypePtr ty = Declarator(&tok, tok, basety, ct);
    res.push_back(ty);
  }
  *rest = tok->SkipToken(";");
  return res;
}

NodePtr Parser::Program(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  tok = tok->SkipToken("{");
  return CompoundStmt(rest, tok, ct);
}

// compound-stmt  = (typedef | declaration | stmt)* "}"
NodePtr Parser::CompoundStmt(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  auto sub_expr = std::make_shared<Node>(ND_END, tok);
  NodePtr cur = sub_expr;

  Scope::EnterScope(scope);

  while (!tok->Equal("}")) {
    // parser declaration.
    // handle conflict between label and typedef.
    if (tok->IsTypename() && !Token::GetNext<1>(tok)->Equal(":")) {
      auto attr = std::make_shared<VarAttr>();
      TypePtr basety = Declspec(&tok, tok, attr, ct);

      if (attr->is_typedef) {
        TypePtrVector ty_list = TypedefDecl(&tok, tok, basety, ct);
        for (auto t : ty_list) {
          Scope::PushVarScope(t->name->GetIdent())->tydef = t;
        }
        continue;
      }
      cur = cur->next = Declaration(&tok, tok, basety, ct);
    }
    // parser statement.
    else {
      cur = cur->next = Stmt(&tok, tok, ct);
    }
    Type::TypeInfer(cur);
  }

  Scope::LevarScope(scope);

  *rest = Token::GetNext<1>(tok);
  return Node::CreateBlockNode(ND_BLOCK, tok, sub_expr->next);
}

// declaration = declspec (
//                 declarator ( "=" expr)?
//                 ("," declarator ("=" expr)? ) * )? ";"
NodePtr Parser::Declaration(TokenPtr* rest, TokenPtr tok, TypePtr basety, ASTree& ct) {
  auto decl_expr = std::make_shared<Node>(ND_END, tok);
  NodePtr cur = decl_expr;
  int i = 0;

  while (!tok->Equal(";")) {
    if (i++ > 0) {
      tok = tok->SkipToken(",");
    }
    TypePtr ty = Declarator(&tok, tok, basety, ct);
    if (ty->Size() < 0) {
      tok->ErrorTok("variable has incomplete type.");
    }
    if (ty->Is<TY_VOID>()) {
      ty->name->ErrorTok("variable declared void.");
    }
    ObjectPtr var = Object::CreateLocalVar(ty->name->GetIdent(), ty, ct.locals);
    if (!tok->Equal("=")) {
      continue;
    }
    NodePtr lhs = Node::CreateVarNode(var, tok);
    NodePtr rhs = Assign(&tok, Token::GetNext<1>(tok), ct);
    NodePtr assgin_node = Node::CreateBinaryNode(ND_ASSIGN, tok, lhs, rhs);
    cur = cur->next = Node::CreateUnaryNode(ND_EXPR_STMT, tok, assgin_node);
  }

  *rest = Token::GetNext<1>(tok);
  return Node::CreateBlockNode(ND_BLOCK, tok, decl_expr->next);
}

// declspec = ("void" | "char" | "int" | "short" | "long"
//            | struct-decl | union-decl )+
//
// The order of typenames in a type-specifier doesn't matter. For
// example, `int long static` means the same as `static long int`.
// That can also be written as `static long` because you can omit
// `int` if `long` or `short` are specified. However, something like
// `char int` is not a valid type specifier. We have to accept only a
// limited combinations of the typenames.
//
// In this function, we count the number of occurrences of each typename
// while keeping the "current" type object that the typenames up
// until that point represent. When we reach a non-typename token,
// we returns the current type object.
TypePtr Parser::Declspec(TokenPtr* rest, TokenPtr tok, VarAttrPtr attr, ASTree& ct) {
  // We use a single integer as counters for all typenames.
  // For example, bits 0 and 1 represents how many times we saw the
  // keyword "void" so far. With this, we can use a switch statement
  // as you can see below.
  enum {
    VOID = 1 << 0,
    BOOL = 1 << 2,
    CHAR = 1 << 4,
    SHORT = 1 << 6,
    INT = 1 << 8,
    LONG = 1 << 10,
    OTHER = 1 << 12,
  };

  TypePtr ty = ty_int;
  int counter = 0;
  while (tok->IsTypename()) {
    // handle strong class specifiers.
    if (tok->Equal("typedef") || tok->Equal("static")) {
      if (attr == nullptr) {
        tok->ErrorTok("storage class specifier is not allow in this context.");
      }

      if (tok->Equal("typedef")) {
        attr->is_typedef = true;
      } else {
        attr->is_static = true;
      }
      if (attr->is_typedef && attr->is_static) {
        tok->ErrorTok("typedef and static may not be used together.");
      }

      tok = Token::GetNext<1>(tok);
      continue;
    }

    // Handle user-define types.
    TypePtr tydef = Scope::FindTypedef(tok);
    if (tok->Equal("struct") || tok->Equal("union") || tok->Equal("enum") || tydef != nullptr) {
      if (counter) {
        break;
      }

      if (tok->Equal("struct")) {
        ty = StructDecl(&tok, Token::GetNext<1>(tok), ct);
      } else if (tok->Equal("union")) {
        ty = UnionDecl(&tok, Token::GetNext<1>(tok), ct);
      } else if (tok->Equal("enum")) {
        ty = EnumDecl(&tok, Token::GetNext<1>(tok), ct);
      } else {
        ty = tydef;
        tok = Token::GetNext<1>(tok);
      }
      counter += OTHER;
      continue;
    }

    if (tok->Equal("void")) {
      counter += VOID;
    } else if (tok->Equal("_Bool")) {
      counter += BOOL;
    } else if (tok->Equal("char")) {
      counter += CHAR;
    } else if (tok->Equal("short")) {
      counter += SHORT;
    } else if (tok->Equal("int")) {
      counter += INT;
    } else if (tok->Equal("long")) {
      counter += LONG;
    } else {
      unreachable();
    }

    switch (counter) {
      case VOID:
        ty = ty_void;
        break;
      case BOOL:
        ty = ty_bool;
        break;
      case CHAR:
        ty = ty_char;
        break;
      case SHORT:
      case SHORT + INT:
        ty = ty_short;
        break;
      case INT:
        ty = ty_int;
        break;
      case LONG:
      case LONG + INT:
      case LONG + LONG:
      case LONG + LONG + INT:
        ty = ty_long;
        break;
      default:
        tok->ErrorTok("invalid type.");
    }
    tok = Token::GetNext<1>(tok);
  }
  *rest = tok;
  return ty;
}

// declarator = "*"* ( "(" ident ")" | "(" declarator ")" | ident)
TypePtr Parser::Declarator(TokenPtr* rest, TokenPtr tok, TypePtr ty, ASTree& ct) {
  while (tok->Equal("*")) {
    ty = Type::CreatePointerType(ty);
    tok = Token::GetNext<1>(tok);
  }

  if (tok->Equal("(")) {
    TokenPtr start = tok;
    auto head = std::make_shared<Type>(TY_END, 0, 0);
    Declarator(&tok, Token::GetNext<1>(start), head, ct);
    tok = tok->SkipToken(")");
    ty = TypeSuffix(rest, tok, ty, ct);
    return Declarator(&tok, Token::GetNext<1>(start), ty, ct);
  }

  if (!tok->Is<TK_IDENT>()) {
    tok->ErrorTok("expected a variable name.");
  }
  ty = TypeSuffix(rest, Token::GetNext<1>(tok), ty, ct);
  ty->name = tok;
  return ty;
}

// array-dimenstion = const-expr ? "]" type-suffix;
TypePtr Parser::ArrayDimention(TokenPtr* rest, TokenPtr tok, TypePtr ty, ASTree& ct) {
  if (tok->Equal("]")) {
    ty = TypeSuffix(rest, Token::GetNext<1>(tok), ty, ct);
    return Type::CreateArrayType(ty, -1);
  }
  int sz = ConstExprEval(&tok, tok, ct);
  tok = tok->SkipToken("]");
  ty = TypeSuffix(rest, tok, ty, ct);
  return Type::CreateArrayType(ty, sz);
}

// type-suffix = "(" func-params ")" |
//               "[" array-dimention | ɛ
TypePtr Parser::TypeSuffix(TokenPtr* rest, TokenPtr tok, TypePtr ty, ASTree& ct) {
  if (tok->Equal("(")) {
    return FunctionParam(rest, Token::GetNext<1>(tok), ty, ct);
  }
  if (tok->Equal("[")) {
    return ArrayDimention(rest, Token::GetNext<1>(tok), ty, ct);
  }
  *rest = tok;
  return ty;
}

TypePtrVector Parser::TypedefDecl(TokenPtr* rest, TokenPtr tok, TypePtr basety, ASTree& ct) {
  auto head = std::make_shared<Type>(TY_END, 1, 1);
  TypePtr cur = head;
  bool first = true;
  TypePtrVector res;
  while (!tok->Equal(";")) {
    if (!first) {
      tok = tok->SkipToken(",");
    }
    first = false;
    res.push_back(Declarator(&tok, tok, basety, ct));
  }
  *rest = tok->SkipToken(";");
  return res;
}

// abstract-declarator = "*"* ("(" abstract-declarator ")")? type-suffix
static TypePtr AbstractDeclarator(TokenPtr* rest, TokenPtr tok, TypePtr ty, ASTree& ct) {
  while (tok->Equal("*")) {
    ty = Type::CreatePointerType(ty);
    tok = Token::GetNext<1>(tok);
  }

  if (tok->Equal("(")) {
    TokenPtr start = tok;
    auto head = std::make_shared<Type>(TY_END, 0, 0);
    AbstractDeclarator(&tok, Token::GetNext<1>(start), head, ct);
    tok = tok->SkipToken(")");
    ty = Parser::TypeSuffix(rest, tok, ty, ct);
    return AbstractDeclarator(&tok, Token::GetNext<1>(start), ty, ct);
  }

  return Parser::TypeSuffix(rest, tok, ty, ct);
}

// typename = declspec abstract-declarator
TypePtr Parser::Typename(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  TypePtr ty = Declspec(&tok, tok, nullptr, ct);
  return AbstractDeclarator(rest, tok, ty, ct);
}

// func-param = param ("," param) *
// param = declspec declarator
TypePtr Parser::FunctionParam(TokenPtr* rest, TokenPtr tok, TypePtr ty, ASTree& ct) {
  auto params = std::make_shared<Type>(TY_END, 0, 0);
  TypePtrVector param;
  bool first = true;

  while (!tok->Equal(")")) {
    if (!first) {
      tok = tok->SkipToken(",");
    }
    first = false;
    TypePtr param_ty = Declspec(&tok, tok, nullptr, ct);
    param_ty = Declarator(&tok, tok, param_ty, ct);

    // "array of T" is converted to pointer of T only in the parameter context.
    if (param_ty->Is<TY_ARRAY>()) {
      TokenPtr name = param_ty->GetName();
      param_ty = Type::CreatePointerType(param_ty->GetBase());
      param_ty->name = name;
    }
    param.push_back(std::make_shared<Type>(*param_ty));
  }
  ty = Type::CreateFunctionType(ty, param);

  *rest = Token::GetNext<1>(tok);
  return ty;
}

static TypePtr StructOrUnionDecl(TypeKind kind, TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  DEBUG(kind == TY_STRUCT || kind == TY_UNION);
  // read struct or union tag.
  TypePtr ty = nullptr;
  TokenPtr tag = nullptr;
  if (tok->Is<TK_IDENT>() == TK_IDENT) {
    tag = tok;
    tok = Token::GetNext<1>(tok);
  }

  if (tag && !tok->Equal("{")) {
    *rest = tok;
    ty = Scope::FindTag(tag->GetIdent());
    if (ty != nullptr) {
      return ty;
    } else {
      // imcompleted type.
      auto ty = std::make_shared<Type>(TY_STRUCT, -1, 1);
      Scope::PushTagScope(tag->GetIdent(), ty);
      return ty;
    }
  }

  MemPtrVector mem = Member::MemberDecl(rest, tok, ct);
  if (kind == TY_STRUCT) {
    ty = Type::CreateStructType(mem, tag);
  } else {
    ty = Type::CreateUnionType(mem, tag);
  }
  if (tag) {
    // If this is a redefinition, overwrite a previous type.
    // Otherwise, register the struct type.
    const String& name = tag->GetIdent();
    auto t = scope->GetTagScope().find(name);
    if (t != scope->GetTagScope().end()) {
      *(t->second) = *ty;
      // avoid cyclic reference
      t->second->UpdateStructMember(mem);
      return t->second;
    }
    Scope::PushTagScope(tag->GetIdent(), ty);
  }
  return ty;
}

TypePtr Parser::StructDecl(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  return StructOrUnionDecl(TY_STRUCT, rest, tok, ct);
}

TypePtr Parser::UnionDecl(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  return StructOrUnionDecl(TY_UNION, rest, tok, ct);
}

// enum-specifier = ident ? "{" enum-list? "}"
//                | ident ( "{" enum-list? "}" )?
// enum-list = ident ("=" const-expr)? ("," ident ("=" const-expr)? )*
TypePtr Parser::EnumDecl(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  TypePtr ty = Type::CreateEnumType();

  TokenPtr tag = nullptr;
  // read a struct tag.
  if (tok->Is<TK_IDENT>()) {
    tag = tok;
    tok = Token::GetNext<1>(tok);
  }
  if (tag && !tok->Equal("{")) {
    *rest = tok;
    TypePtr res = Scope::FindTag(tag->GetIdent());
    if (!res) {
      tok->ErrorTok("unknow enum tag");
    }
    if (!res->Is<TY_ENUM>()) {
      tok->ErrorTok("not a enum tag");
    }
    return res;
  }

  tok = tok->SkipToken("{");
  int i = 0;
  int val = 0;
  while (!tok->Equal("}")) {
    if (i++ > 0) {
      tok = tok->SkipToken(",");
    }

    const String& name = tok->GetIdent();
    tok = Token::GetNext<1>(tok);
    if (tok->Equal("=")) {
      val = ConstExprEval(&tok, Token::GetNext<1>(tok), ct);
    }

    Scope::PushVarScope(name)->SetEnumList(val++, ty);
  }
  *rest = tok->SkipToken("}");
  if (tag) {
    Scope::PushTagScope(tag->GetIdent(), ty);
  }
  return ty;
}

// stmt = "return" expr ";" |
//        "if" "(" expr ")" stmt ("else" stmt)? |
//        "switch" "(" expr ")" stmt
//        "case" const-expr ":" stmt
//        "default" ":" stmt
//        "for" "(" expr-stmt expr? ";" expr? ")" stmt |
//        "while" "(" expr ")" stmt |
//        "goto" ident ";" |
//        "break" ";" |
//        "continue" ";" |
//        "ident" ":" stmt |
//        "{" compuound-stmt |
//        expr-stmt
NodePtr Parser::Stmt(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  if (tok->Equal("return")) {
    NodePtr expr = Expr(&tok, Token::GetNext<1>(tok), ct);
    *rest = tok->SkipToken(";");
    Type::TypeInfer(expr);
    NodePtr cast = Node::CreateCastNode(expr->name, expr, ct.cur_fn->GetType()->return_ty);
    NodePtr node = Node::CreateUnaryNode(ND_RETURN, tok, cast);
    return node;
  }

  if (tok->Equal("if")) {
    TokenPtr node_name = tok;
    tok = Token::GetNext<1>(tok);
    NodePtr cond = Expr(&tok, tok->SkipToken("("), ct);
    NodePtr then = Stmt(&tok, tok->SkipToken(")"), ct);
    NodePtr els = nullptr;
    if (tok->Equal("else")) {
      els = Stmt(&tok, Token::GetNext<1>(tok), ct);
    }
    *rest = tok;
    return Node::CreateIfNode(ND_IF, node_name, cond, then, els);
  }

  if (tok->Equal("switch")) {
    TokenPtr start = tok;
    tok = Token::GetNext<1>(tok)->SkipToken("(");
    NodePtr body = Expr(&tok, tok, ct);
    tok = tok->SkipToken(")");
    // buffer current switch
    NodePtr swt = cur_swt;
    cur_swt = Node::CreateSwitchNode(start, body);
    // buffer break;
    String brk = ct.cur_brk;
    ct.cur_brk = cur_swt->break_label = CreateUniqueName();
    // parse switch body
    cur_swt->then = Stmt(rest, tok, ct);

    ct.cur_brk = brk;
    NodePtr res = cur_swt;
    cur_swt = swt;
    return res;
  }

  if (tok->Equal("case")) {
    if (!cur_swt) {
      tok->ErrorTok("stary case");
    }
    TokenPtr start = tok;
    int64_t v = ConstExprEval(&tok, Token::GetNext<1>(tok), ct);
    tok = tok->SkipToken(":");

    NodePtr res = Node::CreateCaseNode(start, v, Stmt(rest, tok, ct));
    cur_swt->case_nodes.push_back(res);
    return res;
  }

  if (tok->Equal("default")) {
    if (!cur_swt) {
      tok->ErrorTok("stary case");
    }
    TokenPtr start = tok;
    tok = Token::GetNext<1>(tok)->SkipToken(":");

    NodePtr res = Node::CreateDefaultNode(start, Stmt(rest, tok, ct));
    cur_swt->default_node = res;
    return res;
  }

  if (tok->Equal("for")) {
    TokenPtr node_name = tok;
    tok = Token::GetNext<1>(tok)->SkipToken("(");

    NodePtr init = nullptr;
    NodePtr cond = nullptr;
    NodePtr inc = nullptr;

    Scope::EnterScope(scope);

    String brk = ct.cur_brk;
    String cnt = ct.cur_cnt;
    ct.cur_brk = CreateUniqueName();
    ct.cur_cnt = CreateUniqueName();

    if (tok->IsTypename()) {
      TypePtr basety = Declspec(&tok, tok, nullptr, ct);
      init = Declaration(&tok, tok, basety, ct);
    } else {
      init = ExprStmt(&tok, tok, ct);
    }

    if (!tok->Equal(";")) {
      cond = Expr(&tok, tok, ct);
    }
    tok = tok->SkipToken(";");
    if (!tok->Equal(")")) {
      inc = Expr(&tok, tok, ct);
    }
    tok = tok->SkipToken(")");

    NodePtr body = Stmt(rest, tok, ct);

    Scope::LevarScope(scope);
    NodePtr res = Node::CreateForNode(node_name, init, cond, inc, body, ct.cur_brk, ct.cur_cnt);
    ct.cur_brk = brk;
    ct.cur_cnt = cnt;
    return res;
  }

  if (tok->Equal("while")) {
    TokenPtr node_name = tok;
    tok = Token::GetNext<1>(tok)->SkipToken("(");

    NodePtr cond = Expr(&tok, tok, ct);
    tok = tok->SkipToken(")");

    String brk = ct.cur_brk;
    String cnt = ct.cur_cnt;
    ct.cur_brk = CreateUniqueName();
    ct.cur_cnt = CreateUniqueName();

    NodePtr then = Stmt(rest, tok, ct);
    NodePtr res =
        Node::CreateForNode(node_name, nullptr, cond, nullptr, then, ct.cur_brk, ct.cur_cnt);

    ct.cur_brk = brk;
    ct.cur_cnt = cnt;
    return res;
  }

  if (tok->Equal("goto")) {
    NodePtr node = Node::CreateGotoNode(tok, Token::GetNext<1>(tok)->GetIdent());
    *rest = Token::GetNext<2>(tok)->SkipToken(";");
    return node;
  }

  if (tok->Equal("break")) {
    if (ct.cur_brk.empty()) {
      tok->ErrorTok("stray break");
    }
    NodePtr res = Node::CreateGotoNode(tok, ct.cur_brk, false);
    *rest = Token::GetNext<1>(tok)->SkipToken(";");
    return res;
  }

  if (tok->Equal("continue")) {
    if (ct.cur_cnt.empty()) {
      tok->ErrorTok("stray continue");
    }
    NodePtr res = Node::CreateGotoNode(tok, ct.cur_cnt, false);
    *rest = Token::GetNext<1>(tok)->SkipToken(";");
    return res;
  }

  if (tok->Is<TK_IDENT>() && Token::GetNext<1>(tok)->Equal(":")) {
    return Node::CreateGotoLableNode(tok, Stmt(rest, Token::GetNext<2>(tok), ct));
  }

  if (tok->Equal("{")) {
    return CompoundStmt(rest, Token::GetNext<1>(tok), ct);
  }

  return ExprStmt(rest, tok, ct);
}

// expr-stmt = expr ";"
NodePtr Parser::ExprStmt(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  if (tok->Equal(";")) {
    *rest = Token::GetNext<1>(tok);
    return Node::CreateBlockNode(ND_BLOCK, tok, nullptr);
  }
  NodePtr node = Node::CreateUnaryNode(ND_EXPR_STMT, tok, Expr(&tok, tok, ct));
  *rest = tok->SkipToken(";");
  return node;
}

// expr = assign
NodePtr Parser::Expr(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  NodePtr node = Assign(&tok, tok, ct);

  if (tok->Equal(",")) {
    return Node::CreateBinaryNode(ND_COMMON, tok, node, Expr(rest, Token::GetNext<1>(tok), ct));
  }
  *rest = tok;
  return node;
}

// assign = conditional (assign-op assign)?
// assign-op = "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^=" | "<<=" | ">>="
NodePtr Parser::Assign(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  NodePtr node = Conditional(&tok, tok, ct);
  if (tok->Equal("=")) {
    return Node::CreateBinaryNode(ND_ASSIGN, tok, node, Assign(rest, Token::GetNext<1>(tok), ct));
  }
  if (tok->Equal("+=")) {
    return Node::CreateCombinedNode(
        Node::CreateAddNode(tok, node, Assign(rest, Token::GetNext<1>(tok), ct)), ct.locals);
  }
  if (tok->Equal("-=")) {
    return Node::CreateCombinedNode(
        Node::CreateSubNode(tok, node, Assign(rest, Token::GetNext<1>(tok), ct)), ct.locals);
  }

#define CREATE_COMBINE_NODE(op, ND_label)                                                      \
  if (tok->Equal(op)) {                                                                        \
    return Node::CreateCombinedNode(                                                           \
        Node::CreateBinaryNode(ND_label, tok, node, Assign(rest, Token::GetNext<1>(tok), ct)), \
        ct.locals);                                                                            \
  }

  CREATE_COMBINE_NODE("*=", ND_MUL)
  CREATE_COMBINE_NODE("/=", ND_DIV)
  CREATE_COMBINE_NODE("%=", ND_MOD)
  CREATE_COMBINE_NODE("&=", ND_BITAND)
  CREATE_COMBINE_NODE("|=", ND_BITOR)
  CREATE_COMBINE_NODE("^=", ND_BITXOR)
  CREATE_COMBINE_NODE("<<=", ND_SHL)
  CREATE_COMBINE_NODE(">>=", ND_SHR)

#undef CREATE_COMBINE_NODE

  *rest = tok;
  return node;
}

int64_t Parser::ConstExprEval(TokenPtr* rest, TokenPtr tok, ASTree& ast) {
  NodePtr node = Conditional(rest, tok, ast);
  return Node::Eval(node);
}

NodePtr Parser::Conditional(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  NodePtr cond = LogOr(&tok, tok, ct);
  TokenPtr start = tok;
  if (!tok->Equal("?")) {
    *rest = tok;
    return cond;
  }
  NodePtr then = Expr(&tok, Token::GetNext<1>(tok), ct);
  tok = tok->SkipToken(":");
  NodePtr els = Conditional(rest, tok, ct);
  return Node::CreateIfNode(ND_COND, start, cond, then, els);
}

// logor = logand ( "||" logand)
NodePtr Parser::LogOr(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  NodePtr node = LogAnd(&tok, tok, ct);
  while (tok->Equal("||")) {
    TokenPtr start = tok;
    node = Node::CreateBinaryNode(ND_LOGOR, start, node, LogAnd(&tok, Token::GetNext<1>(tok), ct));
  }
  *rest = tok;
  return node;
}

// logand = bitor ( "&&" bitor)
NodePtr Parser::LogAnd(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  NodePtr node = BitOr(&tok, tok, ct);
  while (tok->Equal("&&")) {
    TokenPtr start = tok;
    node = Node::CreateBinaryNode(ND_LOGAND, start, node, BitOr(&tok, Token::GetNext<1>(tok), ct));
  }
  *rest = tok;
  return node;
}

// bitor = bitxor ( "|" bitxor)
NodePtr Parser::BitOr(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  NodePtr node = BitXor(&tok, tok, ct);
  while (tok->Equal("|")) {
    TokenPtr start = tok;
    node = Node::CreateBinaryNode(ND_BITOR, start, node, BitXor(&tok, Token::GetNext<1>(tok), ct));
  }
  *rest = tok;
  return node;
}

// bitxor = bitand ( "^" bitand)
NodePtr Parser::BitXor(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  NodePtr node = BitAnd(&tok, tok, ct);
  while (tok->Equal("^")) {
    TokenPtr start = tok;
    node = Node::CreateBinaryNode(ND_BITXOR, start, node, BitAnd(&tok, Token::GetNext<1>(tok), ct));
  }
  *rest = tok;
  return node;
}

// bitand = equality ( "&" euquality)
NodePtr Parser::BitAnd(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  NodePtr node = Equality(&tok, tok, ct);
  while (tok->Equal("&")) {
    TokenPtr start = tok;
    node =
        Node::CreateBinaryNode(ND_BITAND, start, node, Equality(&tok, Token::GetNext<1>(tok), ct));
  }
  *rest = tok;
  return node;
}

// equality = relational ("==" relational | "!=" relational)
NodePtr Parser::Equality(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  NodePtr node = Relational(&tok, tok, ct);

  for (;;) {
    TokenPtr node_name = tok;
    if (tok->Equal("==")) {
      node = Node::CreateBinaryNode(ND_EQ, node_name, node,
                                    Relational(&tok, Token::GetNext<1>(tok), ct));
      continue;
    }
    if (tok->Equal("!=")) {
      node = Node::CreateBinaryNode(ND_NE, node_name, node,
                                    Relational(&tok, Token::GetNext<1>(tok), ct));
      continue;
    }

    *rest = tok;
    return node;
  }
}

// relational = shirt ("<" shirt | "<=" shirt | ">" shirt | ">=" shirt)
NodePtr Parser::Relational(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  NodePtr node = Shift(&tok, tok, ct);

  for (;;) {
    TokenPtr node_name = tok;

#define CREATE_REATIONAL_NODE(op, lable, node_left, node_right)             \
  if (tok->Equal(op)) {                                                     \
    node = Node::CreateBinaryNode(lable, node_name, node_left, node_right); \
    continue;                                                               \
  }

    CREATE_REATIONAL_NODE("<", ND_LT, node, Shift(&tok, Token::GetNext<1>(tok), ct))
    CREATE_REATIONAL_NODE("<=", ND_LE, node, Shift(&tok, Token::GetNext<1>(tok), ct))
    CREATE_REATIONAL_NODE(">", ND_LT, Shift(&tok, Token::GetNext<1>(tok), ct), node)
    CREATE_REATIONAL_NODE(">=", ND_LE, Shift(&tok, Token::GetNext<1>(tok), ct), node)

#undef CREATE_REATIONAL_NODE

    *rest = tok;
    return node;
  }
}

// shift = add ("<<" add | add ">>" add)*
NodePtr Parser::Shift(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  NodePtr node = Add(&tok, tok, ct);

  for (;;) {
    TokenPtr start = tok;

#define CREATE_SHIFT_NODE(op, lable, node_left, node_right)                 \
  if (tok->Equal(op)) {                                                     \
    node = Node::CreateBinaryNode(lable, node_name, node_left, node_right); \
    continue;                                                               \
  }

    if (tok->Equal("<<")) {
      node = Node::CreateBinaryNode(ND_SHL, start, node, Add(&tok, Token::GetNext<1>(tok), ct));
      continue;
    }
    if (tok->Equal(">>")) {
      node = Node::CreateBinaryNode(ND_SHR, start, node, Add(&tok, Token::GetNext<1>(tok), ct));
      continue;
    }

    *rest = tok;
    return node;
  }
}

// add = mul ("+"mul | "-" mul)
NodePtr Parser::Add(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  NodePtr node = Mul(&tok, tok, ct);

  for (;;) {
    TokenPtr node_name = tok;
    if (tok->Equal("+")) {
      node = Node::CreateAddNode(node_name, node, Mul(&tok, Token::GetNext<1>(tok), ct));
      continue;
    }
    if (tok->Equal("-")) {
      node = Node::CreateSubNode(node_name, node, Mul(&tok, Token::GetNext<1>(tok), ct));
      continue;
    }
    *rest = tok;
    return node;
  }
}

// mul = cast ("*" cast | "/" cast)
NodePtr Parser::Mul(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  NodePtr node = Cast(&tok, tok, ct);

  for (;;) {
    TokenPtr node_name = tok;

#define CREATE_MUL_NODE(op, lable, node_left, node_right)                   \
  if (tok->Equal(op)) {                                                     \
    node = Node::CreateBinaryNode(lable, node_name, node_left, node_right); \
    continue;                                                               \
  }

    CREATE_MUL_NODE("*", ND_MUL, node, Cast(&tok, Token::GetNext<1>(tok), ct))
    CREATE_MUL_NODE("/", ND_DIV, node, Cast(&tok, Token::GetNext<1>(tok), ct))
    CREATE_MUL_NODE("%", ND_MOD, node, Cast(&tok, Token::GetNext<1>(tok), ct))

#undef CREATE_MUL_NODE

    *rest = tok;
    return node;
  }
}

// cast = "(" type-name ")" cast | unary
NodePtr Parser::Cast(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  if (tok->Equal("(") && Token::GetNext<1>(tok)->IsTypename()) {
    TokenPtr start = tok;
    TypePtr ty = Typename(&tok, Token::GetNext<1>(tok), ct);
    tok = tok->SkipToken(")");
    return Node::CreateCastNode(start, Cast(rest, tok, ct), ty);
  }

  return Unary(rest, tok, ct);
}

// unary = ("+" | "-" | "*" | "&") ? cast | postfix
NodePtr Parser::Unary(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  if (tok->Equal("+")) {
    return Cast(rest, Token::GetNext<1>(tok), ct);
  }
#define CREATE_UNARY_NODE(op, label)                                                  \
  if (tok->Equal(op)) {                                                               \
    return Node::CreateUnaryNode(label, tok, Cast(rest, Token::GetNext<1>(tok), ct)); \
  }

  CREATE_UNARY_NODE("-", ND_NEG)
  CREATE_UNARY_NODE("&", ND_ADDR)
  CREATE_UNARY_NODE("*", ND_DEREF)
  CREATE_UNARY_NODE("!", ND_NOT)
  CREATE_UNARY_NODE("~", ND_BITNOT)

#undef CREATE_UNARY_NODE

  // read ++i ==> i+1
  if (tok->Equal("++")) {
    NodePtr binary = Node::CreateAddNode(tok, Unary(rest, Token::GetNext<1>(tok), ct),
                                         Node::CreateConstNode(1, tok));
    return Node::CreateCombinedNode(binary, ct.locals);
  }
  // read --i ==> i-1
  if (tok->Equal("--")) {
    NodePtr binary = Node::CreateSubNode(tok, Unary(rest, Token::GetNext<1>(tok), ct),
                                         Node::CreateConstNode(1, tok));
    return Node::CreateCombinedNode(binary, ct.locals);
  }
  return Postfix(rest, tok, ct);
}

// postfix = primary ("[" Expr "]" | "." ident | "->" ident )*
NodePtr Parser::Postfix(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  NodePtr node = Primary(&tok, tok, ct);

  for (;;) {
    if (tok->Equal("[")) {
      // x[y] is short for *(x + y)
      TokenPtr node_name = tok;
      NodePtr idx = Expr(&tok, Token::GetNext<1>(tok), ct);
      tok = tok->SkipToken("]");
      NodePtr op = Node::CreateAddNode(node_name, node, idx);
      node = Node::CreateUnaryNode(ND_DEREF, node_name, op);
      continue;
    }

    if (tok->Equal(".")) {
      node = Node::CreateMemberNode(node, Token::GetNext<1>(tok));
      tok = Token::GetNext<2>(tok);
      continue;
    }

    if (tok->Equal("->")) {
      // x->y is short for (*x).y
      node = Node::CreateUnaryNode(ND_DEREF, tok, node);
      node = Node::CreateMemberNode(node, Token::GetNext<1>(tok));
      tok = Token::GetNext<2>(tok);
      continue;
    }

    if (tok->Equal("++")) {
      node = Node::CreateIncdecNode(tok, node, 1, ct.locals);
      tok = Token::GetNext<1>(tok);
      continue;
    }

    if (tok->Equal("--")) {
      node = Node::CreateIncdecNode(tok, node, -1, ct.locals);
      tok = Token::GetNext<1>(tok);
      continue;
    }

    *rest = tok;
    return node;
  }
}

// primary = "(" "{" stmt+ "}" ")"
//          |"(" expr ")"
//          | "sizeof" "(" type-name ")"
//          | "sizeof" unary
//          | ident "(" func-args? ")" | str | num
NodePtr Parser::Primary(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  TokenPtr start = tok;

  // This is a GNU statement expression.
  if (tok->Equal("(") && Token::GetNext<1>(tok)->Equal("{")) {
    TokenPtr start = tok;
    NodePtr stmt = CompoundStmt(&tok, Token::GetNext<2>(tok), ct);
    *rest = tok->SkipToken(")");
    return Node::CreateBlockNode(ND_STMT_EXPR, start, stmt->body);
  }
  if (tok->Equal("(")) {
    NodePtr node = Expr(&tok, Token::GetNext<1>(tok), ct);
    *rest = tok->SkipToken(")");
    return node;
  }

  if (tok->Equal("sizeof") && Token::GetNext<1>(tok)->Equal("(") &&
      Token::GetNext<2>(tok)->IsTypename()) {
    TypePtr ty = Typename(&tok, Token::GetNext<2>(tok), ct);
    *rest = tok->SkipToken(")");
    return Node::CreateLongConstNode(ty->Size(), start);
  }

  if (tok->Equal("sizeof")) {
    NodePtr node = Unary(rest, Token::GetNext<1>(tok), ct);
    Type::TypeInfer(node);
    return Node::CreateConstNode(node->ty->size, tok);
  }

  if (tok->Is<TK_IDENT>()) {
    if (Token::GetNext<1>(tok)->Equal("(")) {
      return Call(rest, tok, ct);
    }
    *rest = Token::GetNext<1>(tok);
    return Node::CreateIdentNode(tok);
  }

  if (tok->Is<TK_STR>()) {
    ObjectPtr var = Object::CreateStringVar(String(tok->GetStringLiteral()), ct.globals);
    *rest = Token::GetNext<1>(tok);
    return Node::CreateVarNode(var, tok);
  }

  if (tok->Is<TK_NUM>()) {
    NodePtr node = Node::CreateConstNode(tok->GetNumber(), tok);
    *rest = Token::GetNext<1>(tok);
    return node;
  }

  tok->ErrorTok("expected an expression");
  return nullptr;
}

// function = ident "(" (assign ("," assign)*)? ")"
NodePtr Parser::Call(TokenPtr* rest, TokenPtr tok, ASTree& ct) {
  TokenPtr start = tok;
  // can't optimaze, need start tok.
  tok = Token::GetNext<2>(tok);

  VarScopePtr sc = Scope::FindVarScope(start->GetIdent());
  if (!sc) {
    start->ErrorTok("implicit declaration of a function");
  }
  if (!sc->var || !sc->var->Is<OB_FUNCTION>()) {
    start->ErrorTok("not a function.");
  }

  auto ty = sc->var->GetType();
  auto head = std::make_shared<Node>(ND_END, tok);
  NodePtr cur = head;

  const auto& params_ty = ty->params;
  int pt_idx = 0;

  while (!tok->Equal(")")) {
    if (cur != head) {
      tok = tok->SkipToken(",");
    }
    NodePtr arg = Assign(&tok, tok, ct);
    Type::TypeInfer(arg);

    if (pt_idx < params_ty.size() && params_ty[pt_idx]) {
      if (params_ty[pt_idx]->Is<TY_STRUCT>() || params_ty[pt_idx]->Is<TY_UNION>()) {
        arg->name->ErrorTok("passing struct or union is not support yet");
      }
      arg = Node::CreateCastNode(arg->name, arg, params_ty[pt_idx]);
      ++pt_idx;
    }

    cur = cur->next = arg;
  }
  *rest = tok->SkipToken(")");
  return Node::CreateCallNode(start, head->next, ty);
}