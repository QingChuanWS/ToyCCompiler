#include <iostream>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
    return 1;
  }

  std::cout << ".intel_syntax noprefix" << std::endl;
  std::cout << ".global main" << std::endl;
  std::cout << "main:" << std::endl;
  std::cout << "  mov rax, " << atoi(argv[1]) << std::endl;
  std::cout << "  ret" << std::endl;
  return 0;
}
