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

#include "memory"

class Token;
class Object;
class Node;
class Type;

using TokenPtr = std::shared_ptr<Token>;
using TypePtr = std::shared_ptr<Type>;
using NodePtr = std::shared_ptr<Node>;
using ObjectPtr = std::shared_ptr<Object>;
using StringPtr = std::shared_ptr<std::string>;
using String = std::string;
using StringStream = std::stringstream;

extern ObjectPtr locals;
extern ObjectPtr globals;

extern std::shared_ptr<Type> ty_int;
extern std::shared_ptr<Type> ty_char;

#endif // UTILS_GRUAD