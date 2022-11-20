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

#define DEBUG(expr) assert(expr)
#define DLOG(expr) Log(expr)

// Print error message.
void Error(const char* fmt, ...);
// Reports an error location and exit.
void ErrorAt(char* prg, char* loc, const char* fmt, ...);
// Log message
void Log(const char* fmt, ...);
// compare two string based strncmp
bool StartSwith(const char * q, const char* p, int len);
// check whether current character is alpha
bool IsAlpha(char c);
// check whether current character is alpha or number
bool IsAlnum(char c); 

#endif   // !TOOLS_GRUAD