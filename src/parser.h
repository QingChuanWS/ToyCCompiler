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

#include <cstddef>

#include "utils.h"

class ASTree {
 public:
  // All local variable instance created during parsing are accumulated to this list.
  // each function has self local variable.
  ObjectList locals{};

  // Likewise, global variable are accumulated to this list.
  ObjectList globals{};
};

// parsing token list and generate AST.
class Parser {
 public:
  /*  ---- parse OBJECT ---- */
  // parsing token list and generate AST.
  static ASTree Run(TokenPtr tok);
  // create global variable list based on token list.
  static TypePtrVector ParseGlobalVar(TokenPtr* rest, TokenPtr tok, TypePtr basety, ASTree& ctxt);

  /*  ---- parse TYPE ---- */
  // declspec = ( "_Bool" | "void" | "char" | "int"
  //             | "short" | "long"
  //             | "typedef" | "static" | struct-decl
  //             | union-def | typedef-name | enum-specifier)+
  static TypePtr Declspec(TokenPtr* rest, TokenPtr tok, VarAttrPtr attr, ASTree& ctxt);
  // struct-decl = ident? "{" struct-member
  static TypePtr StructDecl(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // union-decl = ident? "{" union-member
  static TypePtr UnionDecl(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // enum-specifier = ident ? "{" enum-list? "}"
  //                | ident ( "{" enum-list? "}" )?
  // enum-list = ident ("=" num)? ("," ident ("=" num)? )*
  static TypePtr EnumDecl(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // typedef = declspec (ident (",")? ) + ";"
  static TypePtrVector TypedefDecl(TokenPtr* rest, TokenPtr tok, TypePtr basety, ASTree& ctxt);
  // declarator = "*"* ident type-suffix
  static TypePtr Declarator(TokenPtr* rest, TokenPtr tok, TypePtr ty, ASTree& ctxt);
  // type-suffix = "(" func-params | "[" num "]" | ɛ )
  static TypePtr TypeSuffix(TokenPtr* rest, TokenPtr tok, TypePtr ty, ASTree& ctxt);
  // array-dimension = num ? "]" type-suffix
  static TypePtr ArrayDimention(TokenPtr* rest, TokenPtr tok, TypePtr ty, ASTree& ctxt);
  // func-param = param ("," param) *
  // param = declspec declarator
  static TypePtr FunctionParam(TokenPtr* rest, TokenPtr tok, TypePtr ty, ASTree& ctxt);
  // typename = declspec abstract-declarator
  static TypePtr Typename(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);

  /*  ---- parse STATEMENT ---- */
  // program = stmt*
  static NodePtr Program(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // compound-stmt = (typedef | declaration | stmt)* "}"
  static NodePtr CompoundStmt(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // declaration = declspec (
  //                 declarator ( "=" expr)?
  //                 ("," declarator ("=" expr)? ) * )? ";"
  static NodePtr Declaration(TokenPtr* rest, TokenPtr tok, TypePtr basety, ASTree& ctxt);
  // stmt = "return" expr ";" |
  //        "if" "(" expr ")" stmt ("else" stmt)? |
  //        "switch" "(" expr ")" stmt
  //        "case" num ":" stmt
  //        "default" ":" stmt
  //        "for" "(" expr-stmt expr? ";" expr? ")" stmt |
  //        "while" "(" expr ")" stmt |
  //        "goto" ident ";" |
  //        "break" ";" |
  //        "continue" ";" |
  //        "ident" ":" stmt |
  //        "{" compuound-stmt |
  //        expr-stmt
  static NodePtr Stmt(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // expr-stmt = expr ";"
  static NodePtr ExprStmt(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // expr = assign ("," expr)?
  static NodePtr Expr(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // assign = conditional (assign-op assign)?
  // assign-op = "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^=" | "<<=" | ">>="
  static NodePtr Assign(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // conditional = logor ("?" expr : conditional)?
  static NodePtr Conditional(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // logor = logadd ( "||" logadd)
  static NodePtr LogOr(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // logand = bitor ( "&&" bitor)
  static NodePtr LogAnd(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // bitor = bitxor ( "|" bitxor)
  static NodePtr BitOr(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // bitxor = bitand ( "^" bitand)
  static NodePtr BitXor(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // bitand = equality ( "&" euquality)
  static NodePtr BitAnd(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // equality = relational ("==" relational | "!=" relational)
  static NodePtr Equality(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // relational = shirt ("<" shirt | "<=" shirt | ">" shirt | ">=" shirt)
  static NodePtr Relational(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // shift = add ("<<" add | add ">>" add)*
  static NodePtr Shift(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // add = mul ("+"mul | "-" mul)
  static NodePtr Add(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // mul = cast ("*" cast | "/" cast)
  static NodePtr Mul(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // cast = "(" type-name ")" cast | unary
  static NodePtr Cast(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // unary = ("+" | "-" | "*" | "&" | "!" | "~") cast?
  //         | ("++" | "--") unary
  //         | postfix
  static NodePtr Unary(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // postfix = primary ("[" Expr "]" | "." ident | "->" ident )*
  static NodePtr Postfix(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // primary = "(" "{" stmt+ "}" ")"
  //          | "(" expr ")" | "sizeof" unary
  //          | ident "(" func-args? ")" | str | num
  static NodePtr Primary(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
  // function = ident "(" (assign ("," assign)*)? ")"
  static NodePtr Call(TokenPtr* rest, TokenPtr tok, ASTree& ctxt);
};

#endif  // PARSER_GRUAD