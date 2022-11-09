/*
 * This project is exclusively owned by QingChuanWS and shall not be used for
 * commercial and profitting purpose without QingChuanWS's permission.
 * 
 * @Author: bingshan45@163.com
 * Github: https://github.com/QingChuanWS
 * @Description: 
 * 
 * Copyright (c) 2022 by QingChuanWS, All Rights Reserved. 
 */

#include <cstddef>
#include <iostream>
#include <cstdio>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
    return 1;
  }

  char* program = argv[1];

  std::cout << ".intel_syntax noprefix" << std::endl;
  std::cout << ".global main" << std::endl;
  std::cout << "main:" << std::endl;
  std::cout << "  mov rax," << strtol(program, &program, 10) << std::endl;

  while(*program != '\0'){
    if(*program == '+'){
      program++;
      std::cout << "  add rax," << strtol(program, &program, 10) << std::endl;
      continue;
    }
    else if(*program == '-'){
      program++;
      std::cout << "  sub rax," << strtol(program, &program, 10) << std::endl;
      continue;
    }
  }
  std::cout << "  ret" << std::endl;

  return 0;
}
