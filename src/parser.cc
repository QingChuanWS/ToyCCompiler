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

#include <cstddef>
#include <memory>

#include "node.h"
#include "scope.h"
#include "token.h"
#include "tools.h"
#include "type.h"
#include "utils.h"

NodePtr Parser::Program(TokenPtr* rest, TokenPtr tok) {
  tok = tok->SkipToken("{");
  return CompoundStmt(rest, tok);
}

// compound-stmt  = (typedef | declaration | stmt)* "}"
NodePtr Parser::CompoundStmt(TokenPtr* rest, TokenPtr tok) {
  auto sub_expr = std::make_shared<Node>(ND_END, tok);
  NodePtr cur = sub_expr;

  Scope::EnterScope(scope);

  while (!tok->Equal("}")) {
    // parser declaration.
    if (tok->IsTypename()) {
      auto attr = std::make_shared<VarAttr>();
      TypePtr basety = Declspec(&tok, tok, attr);

      if (attr->is_typedef) {
        TypePtr ty_list = TypedefDecl(&tok, tok, basety);
        for (TypePtr t = ty_list; t; t = t->next) {
          Scope::PushVarScope(t->name->GetIdent())->tydef = t;
        }
        continue;
      }
      cur = cur->next = Declaration(&tok, tok, basety);
    }
    // parser statement.
    else {
      cur = cur->next = Stmt(&tok, tok);
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
NodePtr Parser::Declaration(TokenPtr* rest, TokenPtr tok, TypePtr basety) {
  auto decl_expr = std::make_shared<Node>(ND_END, tok);
  NodePtr cur = decl_expr;
  int i = 0;

  while (!tok->Equal(";")) {
    if (i++ > 0) {
      tok = tok->SkipToken(",");
    }
    TypePtr ty = Declarator(&tok, tok, basety);
    if (ty->Is<TY_VOID>()) {
      ty->name->ErrorTok("variable declared void.");
    }
    ObjectPtr var = Object::CreateLocalVar(ty->name->GetIdent(), ty, &locals);
    if (!tok->Equal("=")) {
      continue;
    }
    NodePtr lhs = Node::CreateVarNode(var, tok);
    NodePtr rhs = Assign(&tok, Token::GetNext<1>(tok));
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
TypePtr Parser::Declspec(TokenPtr* rest, TokenPtr tok, VarAttrPtr attr) {
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
      if (attr->is_typedef + attr->is_static > 1) {
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
        ty = StructDecl(&tok, Token::GetNext<1>(tok));
      } else if (tok->Equal("union")) {
        ty = UnionDecl(&tok, Token::GetNext<1>(tok));
      } else if (tok->Equal("enum")) {
        ty = EnumDecl(&tok, Token::GetNext<1>(tok));
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
TypePtr Parser::Declarator(TokenPtr* rest, TokenPtr tok, TypePtr ty) {
  while (tok->Equal("*")) {
    ty = Type::CreatePointerType(ty);
    tok = Token::GetNext<1>(tok);
  }

  if (tok->Equal("(")) {
    TokenPtr start = tok;
    auto head = std::make_shared<Type>(TY_END, 0, 0);
    Declarator(&tok, Token::GetNext<1>(start), head);
    tok = tok->SkipToken(")");
    ty = TypeSuffix(rest, tok, ty);
    return Declarator(&tok, Token::GetNext<1>(start), ty);
  }

  if (!tok->Is<TK_IDENT>()) {
    tok->ErrorTok("expected a variable name.");
  }
  ty = TypeSuffix(rest, Token::GetNext<1>(tok), ty);
  ty->name = tok;
  return ty;
}

// type-suffix = "(" func-params ")" | "[" num "]" type-suffix | ɛ
TypePtr Parser::TypeSuffix(TokenPtr* rest, TokenPtr tok, TypePtr ty) {
  if (tok->Equal("(")) {
    return FunctionParam(rest, Token::GetNext<1>(tok), ty);
  }
  if (tok->Equal("[")) {
    long len = Token::GetNext<1>(tok)->GetNumber();
    tok = Token::GetNext<2>(tok)->SkipToken("]");
    ty = TypeSuffix(rest, tok, ty);
    return Type::CreateArrayType(ty, len);
  }

  *rest = tok;
  return ty;
}

TypePtr Parser::TypedefDecl(TokenPtr* rest, TokenPtr tok, TypePtr basety) {
  auto head = std::make_shared<Type>(TY_END, 1, 1);
  TypePtr cur = head;
  bool first = true;
  while (!tok->Equal(";")) {
    if (!first) {
      tok = tok->SkipToken(",");
    }
    first = false;
    cur = cur->next = Declarator(&tok, tok, basety);
  }
  *rest = tok->SkipToken(";");
  return head->next;
}

// abstract-declarator = "*"* ("(" abstract-declarator ")")? type-suffix
static TypePtr AbstractDeclarator(TokenPtr* rest, TokenPtr tok, TypePtr ty) {
  while (tok->Equal("*")) {
    ty = Type::CreatePointerType(ty);
    tok = Token::GetNext<1>(tok);
  }

  if (tok->Equal("(")) {
    TokenPtr start = tok;
    auto head = std::make_shared<Type>(TY_END, 0, 0);
    AbstractDeclarator(&tok, Token::GetNext<1>(start), head);
    tok = tok->SkipToken(")");
    ty = Parser::TypeSuffix(rest, tok, ty);
    return AbstractDeclarator(&tok, Token::GetNext<1>(start), ty);
  }

  return Parser::TypeSuffix(rest, tok, ty);
}

// typename = declspec abstract-declarator
TypePtr Parser::Typename(TokenPtr* rest, TokenPtr tok) {
  TypePtr ty = Declspec(&tok, tok, nullptr);
  return AbstractDeclarator(rest, tok, ty);
}

// func-param = param ("," param) *
// param = declspec declarator
TypePtr Parser::FunctionParam(TokenPtr* rest, TokenPtr tok, TypePtr ty) {
  auto params = std::make_shared<Type>(TY_END, 0, 0);
  TypePtr cur = params;

  while (!tok->Equal(")")) {
    if (cur != params) {
      tok = tok->SkipToken(",");
    }
    TypePtr base_ty = Declspec(&tok, tok, nullptr);
    TypePtr var_type = Declarator(&tok, tok, base_ty);
    cur->next = std::make_shared<Type>(*var_type);
    cur = cur->next;
  }
  ty = Type::CreateFunctionType(ty, params->next);

  *rest = Token::GetNext<1>(tok);
  return ty;
}

static TypePtr StructOrUnionDecl(TypeKind kind, TokenPtr* rest, TokenPtr tok) {
  DEBUG(kind == TY_STRUCT || kind == TY_UNION);
  // read struct or union tag.
  TokenPtr tag = nullptr;
  TypePtr ty = nullptr;
  if (tok->Is<TK_IDENT>() == TK_IDENT) {
    tag = tok;
    tok = Token::GetNext<1>(tok);
    if (!tok->Equal("{")) {
      ty = Scope::FindTag(tag->GetIdent());
      if (!ty) {
        tok->ErrorTok("unknow struct tag.");
      }
      *rest = tok;
      return ty;
    }
  }

  MemberPtr mem = Member::MemberDecl(rest, tok);
  if (kind == TY_STRUCT) {
    ty = Type::CreateStructType(mem);
  } else {
    ty = Type::CreateUnionType(mem);
  }
  if (tag) {
    scope->GetTagScope()[tag->GetIdent()] = ty;
  }
  return ty;
}

TypePtr Parser::StructDecl(TokenPtr* rest, TokenPtr tok) {
  return StructOrUnionDecl(TY_STRUCT, rest, tok);
}

TypePtr Parser::UnionDecl(TokenPtr* rest, TokenPtr tok) {
  return StructOrUnionDecl(TY_UNION, rest, tok);
}

TypePtr Parser::EnumDecl(TokenPtr* rest, TokenPtr tok) {
  TypePtr ty = Type::CreateEnumType();

  TokenPtr tag = nullptr;
  // read a struct tag.
  if (tok->Is<TK_IDENT>()) {
    tag = tok;
    tok = Token::GetNext<1>(tok);
    if (tag && !tok->Equal("{")) {
      TypePtr res = Scope::FindTag(tag->GetIdent());
      if (!res) {
        tok->ErrorTok("unknow enum tag");
      }
      if (!res->Is<TY_ENUM>()) {
        tok->ErrorTok("not a enum tag");
      }
      *rest = tok;
      return res;
    }
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
      val = Token::GetNext<1>(tok)->GetNumber();
      tok = Token::GetNext<2>(tok);
    }

    Scope::PushVarScope(name)->SetEnumList(val++, ty);
  }
  *rest = tok->SkipToken("}");
  if (tag) {
    scope->GetTagScope()[tag->GetIdent()] = ty;
  }
  return ty;
}

// stmt = "return" expr ";" |
// "if" "(" expr ")" stmt ("else" stmt)? |
// "for" "(" expr-stmt expr? ";" expr? ")" stmt |
// "{" compuound-stmt |
// expr-stmt
NodePtr Parser::Stmt(TokenPtr* rest, TokenPtr tok) {
  if (tok->Equal("return")) {
    NodePtr expr = Expr(&tok, Token::GetNext<1>(tok));
    *rest = tok->SkipToken(";");
    Type::TypeInfer(expr);
    NodePtr cast = Node::CreateCastNode(expr->name, expr, cur_fn->GetType()->return_ty);
    NodePtr node = Node::CreateUnaryNode(ND_RETURN, tok, cast);
    return node;
  }

  if (tok->Equal("if")) {
    TokenPtr node_name = tok;
    tok = Token::GetNext<1>(tok);
    NodePtr cond = Expr(&tok, tok->SkipToken("("));
    NodePtr then = Stmt(&tok, tok->SkipToken(")"));
    NodePtr els = nullptr;
    if (tok->Equal("else")) {
      els = Stmt(&tok, Token::GetNext<1>(tok));
    }
    *rest = tok;
    return Node::CreateIfNode(node_name, cond, then, els);
  }

  if (tok->Equal("for")) {
    TokenPtr node_name = tok;
    tok = Token::GetNext<1>(tok)->SkipToken("(");

    NodePtr init = nullptr;
    NodePtr cond = nullptr;
    NodePtr inc = nullptr;

    Scope::EnterScope(scope);

    if (tok->IsTypename()) {
      TypePtr basety = Declspec(&tok, tok, nullptr);
      init = Declaration(&tok, tok, basety);
    } else {
      init = ExprStmt(&tok, tok);
    }

    if (!tok->Equal(";")) {
      cond = Expr(&tok, tok);
    }
    tok = tok->SkipToken(";");
    if (!tok->Equal(")")) {
      inc = Expr(&tok, tok);
    }
    tok = tok->SkipToken(")");

    NodePtr body = Stmt(rest, tok);

    Scope::LevarScope(scope);
    return Node::CreateForNode(node_name, init, cond, inc, body);
  }

  if (tok->Equal("while")) {
    TokenPtr node_name = tok;
    tok = Token::GetNext<1>(tok)->SkipToken("(");

    NodePtr cond = Expr(&tok, tok);
    tok = tok->SkipToken(")");
    NodePtr then = Stmt(rest, tok);
    return Node::CreateForNode(node_name, nullptr, cond, nullptr, then);
  }

  if (tok->Equal("{")) {
    return CompoundStmt(rest, Token::GetNext<1>(tok));
  }

  return ExprStmt(rest, tok);
}

// expr-stmt = expr ";"
NodePtr Parser::ExprStmt(TokenPtr* rest, TokenPtr tok) {
  if (tok->Equal(";")) {
    *rest = Token::GetNext<1>(tok);
    return Node::CreateBlockNode(ND_BLOCK, tok, nullptr);
  }
  NodePtr node = Node::CreateUnaryNode(ND_EXPR_STMT, tok, Expr(&tok, tok));
  *rest = tok->SkipToken(";");
  return node;
}

// expr = assign
NodePtr Parser::Expr(TokenPtr* rest, TokenPtr tok) {
  NodePtr node = Assign(&tok, tok);

  if (tok->Equal(",")) {
    return Node::CreateBinaryNode(ND_COMMON, tok, node, Expr(rest, Token::GetNext<1>(tok)));
  }
  *rest = tok;
  return node;
}

// assign = equality (assign-op assign)?
// assign-op = "+=" | "-=" | "*=" | "/="
NodePtr Parser::Assign(TokenPtr* rest, TokenPtr tok) {
  NodePtr node = Equality(&tok, tok);
  if (tok->Equal("=")) {
    return Node::CreateBinaryNode(ND_ASSIGN, tok, node, Assign(rest, Token::GetNext<1>(tok)));
  }
  if (tok->Equal("+=")) {
    return Node::CreateCombinedNode(
        Node::CreateAddNode(tok, node, Assign(rest, Token::GetNext<1>(tok))));
  }
  if (tok->Equal("-=")) {
    return Node::CreateCombinedNode(
        Node::CreateSubNode(tok, node, Assign(rest, Token::GetNext<1>(tok))));
  }
  if (tok->Equal("*=")) {
    return Node::CreateCombinedNode(
        Node::CreateBinaryNode(ND_MUL, tok, node, Assign(rest, Token::GetNext<1>(tok))));
  }
  if (tok->Equal("/=")) {
    return Node::CreateCombinedNode(
        Node::CreateBinaryNode(ND_DIV, tok, node, Assign(rest, Token::GetNext<1>(tok))));
  }
  *rest = tok;
  return node;
}

// equality = relational ("==" relational | "!=" relational)
NodePtr Parser::Equality(TokenPtr* rest, TokenPtr tok) {
  NodePtr node = Relational(&tok, tok);

  for (;;) {
    TokenPtr node_name = tok;
    if (tok->Equal("==")) {
      node =
          Node::CreateBinaryNode(ND_EQ, node_name, node, Relational(&tok, Token::GetNext<1>(tok)));
      continue;
    }
    if (tok->Equal("!=")) {
      node =
          Node::CreateBinaryNode(ND_NE, node_name, node, Relational(&tok, Token::GetNext<1>(tok)));
      continue;
    }

    *rest = tok;
    return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)
NodePtr Parser::Relational(TokenPtr* rest, TokenPtr tok) {
  NodePtr node = Add(&tok, tok);

  for (;;) {
    TokenPtr node_name = tok;
    if (tok->Equal("<")) {
      node = Node::CreateBinaryNode(ND_LT, node_name, node, Add(&tok, Token::GetNext<1>(tok)));
      continue;
    }
    if (tok->Equal("<=")) {
      node = Node::CreateBinaryNode(ND_LE, node_name, node, Add(&tok, Token::GetNext<1>(tok)));
      continue;
    }
    if (tok->Equal(">")) {
      node = Node::CreateBinaryNode(ND_LT, node_name, Add(&tok, Token::GetNext<1>(tok)), node);
      continue;
    }
    if (tok->Equal(">=")) {
      node = Node::CreateBinaryNode(ND_LE, node_name, Add(&tok, Token::GetNext<1>(tok)), node);
      continue;
    }
    *rest = tok;
    return node;
  }
}

// add = mul ("+"mul | "-" mul)
NodePtr Parser::Add(TokenPtr* rest, TokenPtr tok) {
  NodePtr node = Mul(&tok, tok);

  for (;;) {
    TokenPtr node_name = tok;
    if (tok->Equal("+")) {
      node = Node::CreateAddNode(node_name, node, Mul(&tok, Token::GetNext<1>(tok)));
      continue;
    }
    if (tok->Equal("-")) {
      node = Node::CreateSubNode(node_name, node, Mul(&tok, Token::GetNext<1>(tok)));
      continue;
    }
    *rest = tok;
    return node;
  }
}

// mul = cast ("*" cast | "/" cast)
NodePtr Parser::Mul(TokenPtr* rest, TokenPtr tok) {
  NodePtr node = Cast(&tok, tok);

  for (;;) {
    TokenPtr node_name = tok;
    if (tok->Equal("*")) {
      node = Node::CreateBinaryNode(ND_MUL, node_name, node, Cast(&tok, Token::GetNext<1>(tok)));
      continue;
    }
    if (tok->Equal("/")) {
      node = Node::CreateBinaryNode(ND_DIV, node_name, node, Cast(&tok, Token::GetNext<1>(tok)));
      continue;
    }
    *rest = tok;
    return node;
  }
}

// cast = "(" type-name ")" cast | unary
NodePtr Parser::Cast(TokenPtr* rest, TokenPtr tok) {
  if (tok->Equal("(") && Token::GetNext<1>(tok)->IsTypename()) {
    TokenPtr start = tok;
    TypePtr ty = Typename(&tok, Token::GetNext<1>(tok));
    tok = tok->SkipToken(")");
    return Node::CreateCastNode(start, Cast(rest, tok), ty);
  }

  return Unary(rest, tok);
}

// unary = ("+" | "-" | "*" | "&") ? cast | postfix
NodePtr Parser::Unary(TokenPtr* rest, TokenPtr tok) {
  if (tok->Equal("+")) {
    return Cast(rest, Token::GetNext<1>(tok));
  }
  if (tok->Equal("-")) {
    return Node::CreateUnaryNode(ND_NEG, tok, Cast(rest, Token::GetNext<1>(tok)));
  }
  if (tok->Equal("&")) {
    return Node::CreateUnaryNode(ND_ADDR, tok, Cast(rest, Token::GetNext<1>(tok)));
  }
  if (tok->Equal("*")) {
    return Node::CreateUnaryNode(ND_DEREF, tok, Cast(rest, Token::GetNext<1>(tok)));
  }
  if (tok->Equal("!")) {
    return Node::CreateUnaryNode(ND_NOT, tok, Cast(rest, Token::GetNext<1>(tok)));
  }
  // read ++i ==> i+1
  if (tok->Equal("++")) {
    NodePtr binary = Node::CreateAddNode(tok, Unary(rest, Token::GetNext<1>(tok)),
                                         Node::CreateConstNode(1, tok));
    return Node::CreateCombinedNode(binary);
  }
  // read --i ==> i-1
  if (tok->Equal("--")) {
    NodePtr binary = Node::CreateSubNode(tok, Unary(rest, Token::GetNext<1>(tok)),
                                         Node::CreateConstNode(1, tok));
    return Node::CreateCombinedNode(binary);
  }
  return Postfix(rest, tok);
}

// postfix = primary ("[" Expr "]" | "." ident | "->" ident )*
NodePtr Parser::Postfix(TokenPtr* rest, TokenPtr tok) {
  NodePtr node = Primary(&tok, tok);

  for (;;) {
    if (tok->Equal("[")) {
      // x[y] is short for *(x + y)
      TokenPtr node_name = tok;
      NodePtr idx = Expr(&tok, Token::GetNext<1>(tok));
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
      node = Node::CreateIncdecNode(tok, node, 1);
      tok = Token::GetNext<1>(tok);
      continue;
    }

    if (tok->Equal("--")) {
      node = Node::CreateIncdecNode(tok, node, -1);
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
NodePtr Parser::Primary(TokenPtr* rest, TokenPtr tok) {
  TokenPtr start = tok;

  // This is a GNU statement expression.
  if (tok->Equal("(") && Token::GetNext<1>(tok)->Equal("{")) {
    TokenPtr start = tok;
    NodePtr stmt = CompoundStmt(&tok, Token::GetNext<2>(tok));
    *rest = tok->SkipToken(")");
    return Node::CreateBlockNode(ND_STMT_EXPR, start, stmt->body);
  }
  if (tok->Equal("(")) {
    NodePtr node = Expr(&tok, Token::GetNext<1>(tok));
    *rest = tok->SkipToken(")");
    return node;
  }

  if (tok->Equal("sizeof") && Token::GetNext<1>(tok)->Equal("(") &&
      Token::GetNext<2>(tok)->IsTypename()) {
    TypePtr ty = Typename(&tok, Token::GetNext<2>(tok));
    *rest = tok->SkipToken(")");
    return Node::CreateLongConstNode(ty->Size(), start);
  }

  if (tok->Equal("sizeof")) {
    NodePtr node = Unary(rest, Token::GetNext<1>(tok));
    Type::TypeInfer(node);
    return Node::CreateConstNode(node->ty->size, tok);
  }

  if (tok->Is<TK_IDENT>()) {
    if (Token::GetNext<1>(tok)->Equal("(")) {
      return Call(rest, tok);
    }
    *rest = Token::GetNext<1>(tok);
    return Node::CreateIdentNode(tok);
  }

  if (tok->Is<TK_STR>()) {
    ObjectPtr var = Object::CreateStringVar(String(tok->GetStringLiteral()));
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
NodePtr Parser::Call(TokenPtr* rest, TokenPtr tok) {
  TokenPtr start = tok;
  // can't optimaze, need start tok.
  tok = Token::GetNext<2>(tok);

  VarScopePtr sc = Scope::FindVarScope(start->GetIdent());
  if (!sc) {
    start->ErrorTok("implicit declaration of a function");
  }
  if (!sc->var || !sc->var->IsFunction()) {
    start->ErrorTok("not a function.");
  }

  TypePtr ty = sc->var->GetType();
  TypePtr param_ty = ty->params;

  auto head = std::make_shared<Node>(ND_END, tok);
  NodePtr cur = head;

  while (!tok->Equal(")")) {
    if (cur != head) {
      tok = tok->SkipToken(",");
    }
    NodePtr arg = Assign(&tok, tok);
    Type::TypeInfer(arg);

    if (param_ty) {
      if (param_ty->Is<TY_STRUCT>() || param_ty->Is<TY_UNION>()) {
        arg->name->ErrorTok("passing struct or union is not support yet");
      }
      arg = Node::CreateCastNode(arg->name, arg, param_ty);
      param_ty = param_ty->next;
    }

    cur = cur->next = arg;
  }
  *rest = tok->SkipToken(")");
  return Node::CreateCallNode(start, head->next, ty);
}
