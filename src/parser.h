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
#include <cstdint>

#include "utils.h"

class ASTree {
 public:
  // All local variable instance created during parsing are accumulated to this list.
  // each function has self local variable.
  ObjectList locals{};

  // Likewise, global variable are accumulated to this list.
  ObjectList globals{};

  // current parsering function.
  ObjectPtr cur_fn = nullptr;

  // Current "goto" and "continue" jump targets.
  String cur_brk = "";
  String cur_cnt = "";
};

/*  ---- parse OBJECT ---- */
class Parser {
 public:
  // parsing token list and generate AST.
  static ASTree Run(TokenPtr tok);
  // create global variable list based on token list.
  static TypePtrVector GlobalVar(TokenPtr* rest, TokenPtr tok, TypePtr basety, ASTree& ast);
  // parsing function
  static TokenPtr GlobalFunction(TokenPtr tok, TypePtr basety, VarAttrPtr attr, ASTree& ast);

  /*  ---- parse TYPE ---- */
  // declspec = ( "_Bool" | "void" | "char" | "int"
  //             | "short" | "long"
  //             | "typedef" | "static" | struct-decl
  //             | union-def | typedef-name | enum-specifier)+
  static TypePtr Declspec(TokenPtr* rest, TokenPtr tok, VarAttrPtr attr, ASTree& ast);
  // struct-decl = ident? "{" struct-member
  static TypePtr StructDecl(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // union-decl = ident? "{" union-member
  static TypePtr UnionDecl(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // enum-specifier = ident ? "{" enum-list? "}"
  //                | ident ( "{" enum-list? "}" )?
  // enum-list = ident ("=" const-expr)? ("," ident ("=" const-expr)? )*
  static TypePtr EnumDecl(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // typedef = declspec (ident (",")? ) + ";"
  static TypePtrVector TypedefDecl(TokenPtr* rest, TokenPtr tok, TypePtr basety, ASTree& ast);
  // declarator = "*"* ident type-suffix
  static TypePtr Declarator(TokenPtr* rest, TokenPtr tok, TypePtr ty, ASTree& ast);
  // type-suffix = "(" func-params | "[" num "]" | ɛ )
  static TypePtr TypeSuffix(TokenPtr* rest, TokenPtr tok, TypePtr ty, ASTree& ast);
  // array-dimension = const-expr ? "]" type-suffix
  static TypePtr ArrayDimention(TokenPtr* rest, TokenPtr tok, TypePtr ty, ASTree& ast);
  // func-param = param ("," param) *
  // param = declspec declarator
  static TypePtr FunctionParam(TokenPtr* rest, TokenPtr tok, TypePtr ty, ASTree& ast);
  // typename = declspec abstract-declarator
  static TypePtr Typename(TokenPtr* rest, TokenPtr tok, ASTree& ast);

  /*  ---- parse STATEMENT ---- */
  // program = stmt*
  static NodePtr Program(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // compound-stmt = (typedef | declaration | stmt)* "}"
  static NodePtr CompoundStmt(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // declaration = declspec (
  //                 declarator ( "=" expr)?
  //                 ("," declarator ("=" expr)? ) * )? ";"
  static NodePtr Declaration(TokenPtr* rest, TokenPtr tok, TypePtr basety, ASTree& ast);
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
  static NodePtr Stmt(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // expr-stmt = expr ";"
  static NodePtr ExprStmt(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // expr = assign ("," expr)?
  static NodePtr Expr(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // assign = conditional (assign-op assign)?
  // assign-op = "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^=" | "<<=" | ">>="
  static NodePtr Assign(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // const_expr = eval(conditional)
  static int64_t ConstExprEval(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // conditional = logor ("?" expr : conditional)?
  static NodePtr Conditional(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // logor = logadd ( "||" logadd)
  static NodePtr LogOr(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // logand = bitor ( "&&" bitor)
  static NodePtr LogAnd(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // bitor = bitxor ( "|" bitxor)
  static NodePtr BitOr(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // bitxor = bitand ( "^" bitand)
  static NodePtr BitXor(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // bitand = equality ( "&" euquality)
  static NodePtr BitAnd(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // equality = relational ("==" relational | "!=" relational)
  static NodePtr Equality(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // relational = shirt ("<" shirt | "<=" shirt | ">" shirt | ">=" shirt)
  static NodePtr Relational(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // shift = add ("<<" add | add ">>" add)*
  static NodePtr Shift(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // add = mul ("+"mul | "-" mul)
  static NodePtr Add(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // mul = cast ("*" cast | "/" cast)
  static NodePtr Mul(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // cast = "(" type-name ")" cast | unary
  static NodePtr Cast(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // unary = ("+" | "-" | "*" | "&" | "!" | "~") cast?
  //         | ("++" | "--") unary
  //         | postfix
  static NodePtr Unary(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // postfix = primary ("[" Expr "]" | "." ident | "->" ident )*
  static NodePtr Postfix(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // primary = "(" "{" stmt+ "}" ")"
  //          | "(" expr ")" | "sizeof" unary
  //          | ident "(" func-args? ")" | str | num
  static NodePtr Primary(TokenPtr* rest, TokenPtr tok, ASTree& ast);
  // function = ident "(" (assign ("," assign)*)? ")"
  static NodePtr Call(TokenPtr* rest, TokenPtr tok, ASTree& ast);
};

#endif  // PARSER_GRUAD