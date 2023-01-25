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
#include "test.h"

int main() {
  ASSERT(8, ({ union { int a; char b[6]; } x; sizeof(x); }));
  ASSERT(3, ({ union { int a; char b[4]; } x; x.a = 515; x.b[0]; }));
  ASSERT(2, ({ union { int a; char b[4]; } x; x.a = 515; x.b[1]; }));
  ASSERT(0, ({ union { int a; char b[4]; } x; x.a = 515; x.b[2]; }));
  ASSERT(0, ({ union { int a; char b[4]; } x; x.a = 515; x.b[3]; }));

  printf("OK\n");
  return 0;
}
