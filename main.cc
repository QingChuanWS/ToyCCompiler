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
#include <cassert>
#include <cctype>
#include <string>

#include "node.h"
#include "token.h"
#include "tools.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
    return 1;
  }

  Token  head;
  Token* cur  = Token::TokenCreate(head, argv[1]);
  Node*  node = Node::Expr(&cur);

  ASM_GEN(".intel_syntax noprefix");
  ASM_GEN(".global main");
  ASM_GEN("main:");

  Node::CodeGen(node);

  ASM_GEN("  pop rax\n");
  ASM_GEN("  ret\n");

  Node::NodeFree(node);
  Token::TokenFree(head);

  return 0;
}
