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
#ifndef UTILS_GRUAD
#define UTILS_GRUAD

#include <stdint.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>
#include <unordered_map>

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
using NodePtr = std::shared_ptr<Node>;
using ObjectPtr = std::shared_ptr<Object>;
using StringPtr = std::shared_ptr<std::string>;
using String = std::string;
using StringStream = std::stringstream;
using ScopePtr = std::shared_ptr<Scope>;
using VarScopePtr = std::shared_ptr<VarScope>;
using TagScopePtr = std::shared_ptr<TagScope>;
using MemberPtr = std::shared_ptr<Member>;
using VarAttrPtr = std::shared_ptr<VarAttr>;
using String = std::string;
using VarScopePtr = std::shared_ptr<VarScope>;
using VarScopeMap = std::unordered_map<String, VarScopePtr>;
using TypedefMap = std::unordered_map<String, TypePtr>;
using TagScopeMap = std::unordered_map<String, TypePtr>;

extern ObjectPtr locals;
extern ObjectPtr globals;

extern ScopePtr scope;

struct VarAttr {
  VarAttr() = default;
  bool is_typedef = false;
};

struct Config {
  Config() = default;
  String output_path = "-";
  String input_path = "-";
};

extern TypePtr ty_void;
extern TypePtr ty_char;
extern TypePtr ty_short;
extern TypePtr ty_int;
extern TypePtr ty_long;

extern Config config;

#endif  // UTILS_GRUAD