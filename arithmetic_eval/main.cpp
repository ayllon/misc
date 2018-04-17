#include <iostream>
#include "ArithmeticEval/Evaluator.h"
#include "ArithmeticEval/Functions.h"


void eval(std::string const &expr, std::map<std::string, double> const &vars = {}) {
  try {
    std::cout << "Expression: " << expr << std::endl;
    Arithmetic::Evaluator evaluator(expr, Arithmetic::AllFunctions());
    std::cout << "Postfix:    " << evaluator.repr() << std::endl;
    std::cout << evaluator(vars) << std::endl;
  }
  catch (std::exception const &e) {
    std::cout << "Error: " << e.what() << std::endl;
  }
  std::cout << std::endl;
}

int main() {
  eval("5");
  eval("5+3*2");
  eval("ID", {{"ID", 42}});
  eval("true && false");
  eval("(5+3)*2");
  eval("(5+3)*2 == 11");
  eval("5+3)*2");
  eval("5+ID");
  eval("5+ID", {{"ID", 42}});
  eval("sqrt 66");
  eval("3+sqrt(ID)", {{"ID", 42}});
  eval("sqrt");
  eval("!(true && true) || !false");
  return 0;
}
