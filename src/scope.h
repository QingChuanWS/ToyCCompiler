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
#ifndef SCOPE_GRUAD
#define SCOPE_GRUAD

#include "object.h"
#include "tools.h"
#include "utils.h"

struct VarScope {
 public:
  VarScope() = default;
  // for variable
  ObjectPtr var{nullptr};
  // for tydef
  TypePtr tydef{nullptr};

  void SetEnumList(int e_val, TypePtr ty) {
    enum_val = e_val;
    enum_ty = ty;
  }
  int GetEnumList() {
    if (!enum_ty) {
      unreachable();
    }
    return enum_val;
  }
  bool IsEnum() { return enum_ty != nullptr; }

 private:
  // for enum
  TypePtr enum_ty{nullptr};
  int enum_val{-1};
};

// C has two block scopes; one is for variables/typedefs and
// the other is for struct/union/enum tags.
class Scope {
 public:
  // get current var scope
  VarScopeMap& GetVarScope() { return vars; }
  // get current tag scope
  TagScopeMap& GetTagScope() { return tags; }

  // create a scpoe
  static void EnterScope(ScopePtr& next);
  // delete a scope
  static void LevarScope(ScopePtr& next);
  // find a variable by name.
  static VarScopePtr FindVarScope(const String& name);
  // find a tag by name.
  static TypePtr FindTag(const String& name);
  // find a typedef name by name.
  static const TypePtr FindTypedef(const TokenPtr& tok);
  // create a varscope.
  static VarScopePtr& PushVarScope(const String& name);

 private:
  // scope link list.
  ScopePtr next = nullptr;
  // Scope for local variables, global variables, typedefs
  // or enum constants.
  VarScopeMap vars = VarScopeMap();
  // Scope for struct, union or enum tags
  TagScopeMap tags = TagScopeMap();
};

#endif  // SCOPE_GRUAD