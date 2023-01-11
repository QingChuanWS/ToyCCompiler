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
template <typename Printer, typename T>
static Printer& Println(Printer& print, const T arg) {
  print << arg;
  return print;
}

template <typename Printer, typename T, typename... Types>
static Printer& Println(Printer& print, const T first_arg, const Types... args) {
  print << first_arg;
  Println(print, args...);
  return print;
}

// Print error message.
void Error(const char* fmt, ...);
// Reports an error location and exit.
void ErrorAt(char* prg, char* loc, const char* fmt, ...);
// Log message.
void Log(const char* fmt, ...);
// compare two string based strncmp.
bool StrEqual(const char* src, const char* dst, int src_len);
// check whether current character is alpha.
bool IsAlpha(char c);
// check whether current character is alpha or number.
bool IsAlnum(char c);
// round up `n` to the nearest multiple of `align`.
int AlignTo(int n, int align);
// create a unique name.
String CreateUniqueName();

#endif  // !TOOLS_GRUAD
