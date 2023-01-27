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

#include "struct.h"

#include <memory>

#include "parser.h"
#include "tools.h"
#include "type.h"
#include "utils.h"

int Member::CalcuStructAlign(MemberPtr mem) {
  int align = 1;
  for (MemberPtr m = mem; m != nullptr; m = m->next) {
    if (align < m->ty->GetAlign()) {
      align = m->ty->GetAlign();
    }
  }
  return align;
}

int Member::CalcuStructOffset(MemberPtr mem) {
  int offset = 0;
  for (MemberPtr m = mem; m != nullptr; m = m->next) {
    offset = AlignTo(offset, m->ty->GetAlign());
    m->offset = offset;
    offset += m->ty->Size();
  }
  return offset;
}
