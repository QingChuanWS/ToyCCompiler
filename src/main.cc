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

static String opt_out;
static String input_path;

static void Usage(int state) {
  std::cerr << "toyc [ -o <path> ] <file>." << std::endl;
  exit(state);
}

static void ParseArgs(int argc, char** argv) {
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--help")) {
      Usage(0);
    }
    if (!strcmp(argv[i], "-o")) {
      if (!argv[++i]) {
        Usage(1);
      }
      std::cout << opt_out;
      opt_out = String(argv[i]);
      continue;
    }
    if (!strncmp(argv[i], "-o", 2)) {
      opt_out = String(argv[i] + 2);
      continue;
    }
    if (argv[i][0] == '-' && argv[i][1] != '\0') {
      Error("unknow argument: %s", argv[i]);
    }

    input_path = argv[i];
  }
  if (input_path.empty()) {
    Error("no input files.");
  }
}

int main(int argc, char** argv) {
  ParseArgs(argc, argv);

  TokenPtr cur = Token::TokenizeFile(input_path);
  ObjectPtr prog = Object::Parse(cur);

  CodeGenerator gene(input_path, opt_out);
  gene.CodeGen(prog);

  return 0;
}
