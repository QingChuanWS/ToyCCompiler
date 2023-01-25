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

#ifndef TOOLS_GRUAD
#define TOOLS_GRUAD

#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>

#include "utils.h"

#define DLOG(...) DebugLog(__VA_ARGS__)
#define DEBUG(expr) assert(expr)
#define unreachable() Error("internal error at %s:%d", __FILE__, __LINE__)

// recursive over state.
template <typename F, typename T>
static void Println(const T arg) {
  F::Print(arg);
}

template <typename F, typename T, typename... Types>
static void Println(const T first_arg, const Types... args) {
  F::Print(first_arg);
  Println<F>(args...);
}

// Print error message.
void Error(const char* fmt, ...);
// Log message.
void DebugLog(const char* fmt, ...);
// compare two string based strncmp.
bool StrEqual(const char* src, const char* dst, const int src_len);
// check whether current character is alpha.
inline bool IsAlpha(const char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}
// check whether current character is alpha or number.
inline bool IsAlnum(const char c) { return IsAlpha(c) || ('0' <= c && c <= '9'); }
// round up `n` to the nearest multiple of `align`.
inline int AlignTo(const int n, const int align) { return (n + align - 1) / align * align; }
// create a unique name.
String CreateUniqueName();
// compiler helper function.
void Usage(int state);
// parse input arguement.
Config ParseArgs(int argc, char** argv);

#endif  // !TOOLS_GRUAD
