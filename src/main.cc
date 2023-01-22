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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <ostream>

#include "codegen.h"
#include "node.h"
#include "object.h"
#include "token.h"
#include "tools.h"
#include "utils.h"

Config config;

int main(int argc, char** argv) {
  ParseArgs(argc, argv);

  TokenPtr cur = Token::TokenizeFile(config.input_path);
  ObjectPtr prog = Object::Parse(cur);

  CodeGenerator gene(config);
  gene.CodeGen(prog);

  return 0;
}
