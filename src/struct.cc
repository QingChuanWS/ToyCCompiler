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

#include "parser.h"
#include "tools.h"
#include "type.h"
#include "utils.h"

int Member::CalcuStructAlign(const MemPtrVector& mem) {
  int align = 1;
  for (auto m : mem) {
    if (align < m->ty->GetAlign()) {
      align = m->ty->GetAlign();
    }
  }
  return align;
}

int Member::CalcuStructOffset(const MemPtrVector& mem) {
  int offset = 0;
  for (MemberPtr m : mem) {
    offset = AlignTo(offset, m->ty->GetAlign());
    m->offset = offset;
    offset += m->ty->Size();
  }
  return offset;
}

// struct-decl = "{" struct or union member
MemPtrVector Member::MemberDecl(TokenPtr* rest, TokenPtr tok) {
  tok = tok->SkipToken("{");
  auto head = std::make_shared<Member>();
  MemPtrVector mem_vec;

  while (!tok->Equal("}")) {
    TypePtr basety = Parser::Declspec(&tok, tok, nullptr);

    bool first = true;
    while (!tok->Equal(";")) {
      if (!first) {
        tok = tok->SkipToken(",");
      }
      first = false;
      TypePtr ty = Parser::Declarator(&tok, tok, basety);
      mem_vec.push_back(std::make_shared<Member>(ty, ty->GetName()));
    }
    tok = tok->SkipToken(";");
  }
  *rest = Token::GetNext<1>(tok);
  return mem_vec;
}
