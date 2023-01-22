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
#include "struct.h"

#include <memory>

#include "parser.h"
#include "type.h"
#include "utils.h"

int Struct::GetSize(StructPtr mem) {
  int off = 0;
  for (StructPtr m = mem; m != nullptr; m = m->next) {
    m->offset = off;
    off += m->ty->Size();
  }
  return off;
}
