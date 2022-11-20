#ifndef FUNCTION_GRUAD
#define FUNCTION_GRUAD

#include "node.h"
#include "var.h"

struct Function {
  Function(Node* node, Var* var_head = nullptr)
      : node_(node)
      , var_head_(var_head)
      , stack_size_(0) {}

  // parsing token list and generate AST.
  static Function Program(Token** tok);

  void FunctionFree();
  void OffsetCal();

  Node* node_;
  Var*  var_head_;
  int   stack_size_;
};


#endif   // !FUNCTION_GRUAD
