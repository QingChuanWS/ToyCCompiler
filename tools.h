/*
 * This project is exclusively owned by QingChuanWS and shall not be used for
 * commercial and profitting purpose without QingChuanWS's permission.
 *
 * @Author: bingshan45@163.com
 * Github: https://github.com/QingChuanWS
 * @Description:
 *
 * Copyright (c) 2022 by QingChuanWS, All Rights Reserved.
 */

#ifndef TOOLS_GRUAD
#define TOOLS_GRUAD

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <iostream>

#define ASM_GEN(...) ASMGenerator(__VA_ARGS__)
#define DEBUG(expr) assert(expr)
#define DLOG(expr) Log(expr)

// recursive over state.
template<typename T>
void ASMGenerator(const T arg) {
  std::cout << arg << std::endl;
}

template<typename T, typename... Types>
void ASMGenerator(const T first_arg, const Types... args) {
  std::cout << first_arg;
  ASMGenerator(args...);
}

// Print error message.
void Error(const char* fmt, ...);
// Reports an error location and exit.
void ErrorAt(char* prg, char* loc, const char* fmt, ...);
// Log message
void Log(const char* fmt, ...);
// compare two string based strncmp
bool StartSwith(const char * q, const char* p, int len);

#endif   // !TOOLS_GRUAD
