/*
 * This project is exclusively owned by QingChuanWS and shall not be used for
 * commercial and profitting purpose without QingChuanWS's permission.
 *
 * @Author: git config user.email
 * Github: https://github.com/QingChuanWS
 * @Description:
 *
 * Copyright (c) 2022 by ${git_name}, All Rights Reserved.
 */
#include "tools.h"

void Error(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(0);
}

void ErrorAt(char* prg, char* loc, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - prg;
  fprintf(stderr, "%s\n", prg);
  fprintf(stderr, "%*s", pos, "");   // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void LOG(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}