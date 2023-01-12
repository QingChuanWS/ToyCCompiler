/*
 * This project is exclusively owned by QingChuanWS and shall not be used for
 * commercial and profitting purpose without QingChuanWS's permission.
 *
 * @Author: bingshan45@163.com
 * Github: https://github.com/QingChuanWS
 * @Description:
 *
 * Copyright (c) 2023 by QingChuanWS, All Rights Reserved.
 */

#include <cstddef>
#include <memory>

#include "codegen.h"
#include "node.h"
#include "object.h"
#include "token.h"
#include "tools.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
    return 1;
  }

  TokenPtr cur = Token::TokenizeFile(String(argv[1]));
  ObjectPtr prog = Object::Parse(cur);

  CodeGenerator gene;
  gene.CodeGen(prog);
  
  return 0;
}
