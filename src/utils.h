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

#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>

class Token;
class Object;
class Node;
class Type;
class VarScope;
class Scope;
class Struct;

using TokenPtr = std::shared_ptr<Token>;
using TypePtr = std::shared_ptr<Type>;
using NodePtr = std::shared_ptr<Node>;
using ObjectPtr = std::shared_ptr<Object>;
using StringPtr = std::shared_ptr<std::string>;
using String = std::string;
using StringStream = std::stringstream;
using ScopePtr = std::shared_ptr<Scope>;
using VarScopePtr = std::shared_ptr<VarScope>;
using StructPtr = std::shared_ptr<Struct>;
using String = std::string;

extern ObjectPtr locals;
extern ObjectPtr globals;

extern ScopePtr scope;

extern std::shared_ptr<Type> ty_int;
extern std::shared_ptr<Type> ty_char;

struct Config {
  Config() = default;
  String output_path = "-";
  String input_path = "-";
};

#endif  // UTILS_GRUAD