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

#ifndef PARSER_GRUAD
#define PARSER_GRUAD

#include "utils.h"

// parsing token list and generate AST.
class Parser {
 public:
  // ---- parse TYPE ----
  // declspec = ( "_Bool" | "void" | "char" | "int"
  //             | "short" | "long"
  //             | "typedef" | "static" | struct-decl
  //             | union-def | typedef-name | enum-specifier)+
  static TypePtr Declspec(TokenPtr* rest, TokenPtr tok, VarAttrPtr attr);
  // struct-decl = ident? "{" struct-member
  static TypePtr StructDecl(TokenPtr* rest, TokenPtr tok);
  // union-decl = ident? "{" union-member
  static TypePtr UnionDecl(TokenPtr* rest, TokenPtr tok);
  // enum-specifier = ident ? "{" enum-list? "}"
  //                | ident ( "{" enum-list? "}" )?
  // enum-list = ident ("=" num)? ("," ident ("=" num)? )*
  static TypePtr EnumDecl(TokenPtr* rest, TokenPtr tok);
  // typedef = declspec (ident (",")? ) + ";"
  static TypePtr TypedefDecl(TokenPtr* rest, TokenPtr tok, TypePtr basety);
  // declarator = "*"* ident type-suffix
  static TypePtr Declarator(TokenPtr* rest, TokenPtr tok, TypePtr ty);
  // type-suffix = "(" func-params | "[" num "]" | ɛ )
  static TypePtr TypeSuffix(TokenPtr* rest, TokenPtr tok, TypePtr ty);
  // func-param = param ("," param) *
  // param = declspec declarator
  static TypePtr FunctionParam(TokenPtr* rest, TokenPtr tok, TypePtr ty);
  // typename = declspec abstract-declarator
  static TypePtr Typename(TokenPtr* rest, TokenPtr tok);

  // ---- parse Statemet ----
  // program = stmt*
  static NodePtr Program(TokenPtr* rest, TokenPtr tok);
  // compound-stmt = (typedef | declaration | stmt)* "}"
  static NodePtr CompoundStmt(TokenPtr* rest, TokenPtr tok);
  // declaration = declspec (
  //                 declarator ( "=" expr)?
  //                 ("," declarator ("=" expr)? ) * )? ";"
  static NodePtr Declaration(TokenPtr* rest, TokenPtr tok, TypePtr basety);
  // stmt = "return" expr ";" |
  //        "if" "(" expr ")" stmt ("else" stmt)? |
  //        "for" "(" expr-stmt expr? ";" expr? ")" stmt |
  //        "while" "(" expr ")" stmt |
  //        "{" compuound-stmt |
  // expr-stmt
  static NodePtr Stmt(TokenPtr* rest, TokenPtr tok);
  // expr-stmt = expr ";"
  static NodePtr ExprStmt(TokenPtr* rest, TokenPtr tok);
  // expr = assign ("," expr)?
  static NodePtr Expr(TokenPtr* rest, TokenPtr tok);
  // assign = logor (assign-op assign)?
  // assign-op = "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^="
  static NodePtr Assign(TokenPtr* rest, TokenPtr tok);
  // logor = logadd ( "||" logadd)
  static NodePtr LogOr(TokenPtr* rest, TokenPtr tok);
  // logand = bitor ( "&&" bitor)
  static NodePtr LogAnd(TokenPtr* rest, TokenPtr tok);
  // bitor = bitxor ( "|" bitxor)
  static NodePtr BitOr(TokenPtr* rest, TokenPtr tok);
  // bitxor = bitand ( "^" bitand)
  static NodePtr BitXor(TokenPtr* rest, TokenPtr tok);
  // bitand = equality ( "&" euquality)
  static NodePtr BitAnd(TokenPtr* rest, TokenPtr tok);
  // equality = relational ("==" relational | "!=" relational)
  static NodePtr Equality(TokenPtr* rest, TokenPtr tok);
  // relational = add ("<" add | "<=" add | ">" add | ">=" add)
  static NodePtr Relational(TokenPtr* rest, TokenPtr tok);
  // add = mul ("+"mul | "-" mul)
  static NodePtr Add(TokenPtr* rest, TokenPtr tok);
  // mul = cast ("*" cast | "/" cast)
  static NodePtr Mul(TokenPtr* rest, TokenPtr tok);
  // cast = "(" type-name ")" cast | unary
  static NodePtr Cast(TokenPtr* rest, TokenPtr tok);
  // unary = ("+" | "-" | "*" | "&" | "!" | "~") cast?
  //         | ("++" | "--") unary
  //         | postfix
  static NodePtr Unary(TokenPtr* rest, TokenPtr tok);
  // postfix = primary ("[" Expr "]" | "." ident | "->" ident )*
  static NodePtr Postfix(TokenPtr* rest, TokenPtr tok);
  // primary = "(" "{" stmt+ "}" ")"
  //          | "(" expr ")" | "sizeof" unary
  //          | ident "(" func-args? ")" | str | num
  static NodePtr Primary(TokenPtr* rest, TokenPtr tok);
  // function = ident "(" (assign ("," assign)*)? ")"
  static NodePtr Call(TokenPtr* rest, TokenPtr tok);
};

#endif  // PARSER_GRUAD