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

#include <cstdio>
#include <cstdlib>

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
  fprintf(stderr, "%*s", pos, "");  // print pos spaces.
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

bool StrEqual(const char* src, const char* dst, int src_len) {
  return memcmp(src, dst, src_len) == 0 && dst[src_len] == '\0';
}

bool IsAlpha(char c) { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_'; }

bool IsAlnum(char c) { return IsAlpha(c) || ('0' <= c && c <= '9'); }

int AlignTo(int n, int align) { return (n + align - 1) / align * align; }

char* CreateUniqueName() {
  static int id = 0;
  char* buf = (char*)malloc(20 * sizeof(char));
  sprintf(buf, ".L..%d", id++);
  return buf;
}