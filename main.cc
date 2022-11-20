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
#include "token.h"
#include "tools.h"

#include <cassert>
#include <cctype>
#include <string>

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
    return 1;
  }

  Token head = Token();
  Token* cur  = Token::TokenCreate(head, argv[1]);
  Node*  node = Node::Program(&cur);

  CodeGenerator gene;
  gene.CodeGen(node);

  Token::TokenFree(head);
  Node::NodeFree(node);

  return 0;
}
