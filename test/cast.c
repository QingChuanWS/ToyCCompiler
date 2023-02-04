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

#include "test.h"

int main() {
  ASSERT(131585, (int)8590066177);
  ASSERT(513, (short)8590066177);
  ASSERT(1, (char)8590066177);
  ASSERT(1, (long)1);
  ASSERT(0, (long)&*(int *)0);
  ASSERT(513, ({ int x=512; *(char *)&x=1; x; }));
  ASSERT(5, ({ int x=5; long y=(long)&x; *(int*)y; }));

  (void)1;

  printf("OK\n");
  return 0;
}
