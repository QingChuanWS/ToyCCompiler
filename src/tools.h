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
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>

using String = std::string;

#define DEBUG(expr) assert(expr)
#define DLOG(expr) Log(expr)

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
void Log(const char* fmt, ...);
// compare two string based strncmp.
bool StrEqual(const char* src, const char* dst, const int src_len);
// check whether current character is alpha.
bool IsAlpha(const char c);
// check whether current character is alpha or number.
bool IsAlnum(const char c);
// round up `n` to the nearest multiple of `align`.
int AlignTo(const int n, const int align);
// create a unique name.
const String CreateUniqueName();

#endif  // !TOOLS_GRUAD