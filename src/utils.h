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

#ifndef UTILS_GRUAD
#define UTILS_GRUAD

#include <stdint.h>

#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>
#include <unordered_map>
#include <vector>

class Token;
class Object;
class Node;
class Type;
class TagScope;
class VarScope;
class Scope;
class Member;
struct VarAttr;

using TokenPtr = std::shared_ptr<Token>;
using TypePtr = std::shared_ptr<Type>;
using TypeWeakPtr = std::weak_ptr<Type>;
using TypePtrVector = std::vector<TypePtr>;
using NodePtr = std::shared_ptr<Node>;
using NodePtrVec = std::vector<NodePtr>;
using ObjectPtr = std::shared_ptr<Object>;
using StringPtr = std::shared_ptr<std::string>;
using String = std::string;
using StringStream = std::stringstream;
using ScopePtr = std::shared_ptr<Scope>;
using VarScopePtr = std::shared_ptr<VarScope>;
using TagScopePtr = std::shared_ptr<TagScope>;
using MemberPtr = std::shared_ptr<Member>;
using MemPtrVector = std::vector<MemberPtr>;
using VarAttrPtr = std::shared_ptr<VarAttr>;
using String = std::string;
using VarScopePtr = std::shared_ptr<VarScope>;
using VarScopeMap = std::unordered_map<String, VarScopePtr>;
using TypedefMap = std::unordered_map<String, TypePtr>;
using TagScopeMap = std::unordered_map<String, TypePtr>;
using ObjectList = std::vector<ObjectPtr>;

template <typename T>
using Matrix = std::vector<std::vector<T>>;

struct VarAttr {
  VarAttr() = default;
  bool is_typedef = false;
  bool is_static = false;
};

struct Config {
  Config() = default;
  String output_path = "-";
  String input_path = "-";
};

extern ObjectPtr locals;
extern ObjectPtr globals;

extern ObjectPtr cur_fn;
extern NodePtrVec goto_list;
extern NodePtrVec label_list;
extern String cur_brk;
extern String cur_cnt;
extern NodePtr cur_swt;

extern ScopePtr scope;

extern TypePtr ty_void;
extern TypePtr ty_char;
extern TypePtr ty_short;
extern TypePtr ty_int;
extern TypePtr ty_long;
extern TypePtr ty_bool;

extern Config config;

#endif  // UTILS_GRUAD