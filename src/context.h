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
#ifndef CONTEXT_GRUAD
#define CONTEXT_GRUAD

#include "utils.h"

class ASTree {
 public:
  // All local variable instance created during parsing are accumulated to this list.
  // each function has self local variable.
  ObjectList locals{};

  // Likewise, global variable are accumulated to this list.
  ObjectList globals{};
};

#endif