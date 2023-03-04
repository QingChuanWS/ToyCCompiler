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

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <ostream>

#include "codegen.h"
#include "node.h"
#include "object.h"
#include "parser.h"
#include "token.h"
#include "tools.h"
#include "utils.h"

int main(int argc, char** argv) {
  // parsing input arguement.
  Config cfg = ParseArgs(argc, argv);
  // read source code file and generate token list.
  TokenPtr cur = Token::TokenizeFile(cfg.input_path);
  // parse token list generate AST.
  ASTree astree = Parser::Run(cur);
  // config code generator.
  CodeGenerator gene(cfg);
  // generate source code.
  gene.CodeGen(astree);

  return 0;
}
