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

#include "tools.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <sstream>

#include "utils.h"

void Error(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(0);
}

void DebugLog(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}

bool StrEqual(const char* src, const char* dst, const int src_len) {
  return memcmp(src, dst, src_len) == 0 && dst[src_len] == '\0';
}

#define StringFormat(...) Println<StringFormator>(__VA_ARGS__)

class StringFormator {
 public:
  template <typename T>
  static void Print(const T t) {
    GetInstance().sprint << t;
  }

  static StringFormator& GetInstance() {
    static StringFormator printor;
    return printor;
  }
  static String GetString() {
    String res = String(GetInstance().sprint.str());
    GetInstance().sprint.clear();
    GetInstance().sprint.str("");
    return res;
  }

 private:
  StringFormator() {}
  ~StringFormator() {}
  StringFormator(const StringFormator&) = delete;
  StringFormator operator=(const StringFormator&) = delete;
  std::stringstream sprint;
};

String CreateUniqueName() {
  static int id = 0;
  StringFormat(".L..", id++);
  return StringFormator::GetString();
}

// compiler helper function.
void Usage(int state) {
  std::cerr << "toyc [ -o <path> ] <file>." << std::endl;
  exit(state);
}

// parse input arguement.
Config ParseArgs(int argc, char** argv) {
  Config config;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--help")) {
      Usage(0);
    }
    if (!strcmp(argv[i], "-o")) {
      if (!argv[++i]) {
        Usage(1);
      }
      config.output_path = String(argv[i]);
      continue;
    }
    if (!strncmp(argv[i], "-o", 2)) {
      config.output_path = String(argv[i] + 2);
      continue;
    }
    if (argv[i][0] == '-' && argv[i][1] != '\0') {
      Error("unknow argument: %s", argv[i]);
    }

    config.input_path = argv[i];
  }
  if (config.input_path.empty()) {
    Error("no input files.");
  }
  return config;
}
