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

#include "tools.h"

#include <bits/types/FILE.h>

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <sstream>

void Error(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(0);
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

#define StringFormat(...) Println<StringFormator>(__VA_ARGS__)

class StringFormator {
 public:
  template <typename T>
  void operator()(T t) {
    sprint << t;
  }
  static StringFormator& GetInstance(const String& path = "-") {
    static StringFormator printor;
    return printor;
  }
  static const String GetString() { return std::move(GetInstance().sprint.str()); }

 private:
  StringFormator() {}
  ~StringFormator() {}
  StringFormator(const StringFormator&) = delete;
  StringFormator operator=(const StringFormator&) = delete;
  std::stringstream sprint;
};

const String CreateUniqueName() {
  static int id = 0;
  StringFormat(".L..", id++);
  return StringFormator::GetString();
}