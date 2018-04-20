#ifndef ARITHMETIC_EVAL_PARSER_H
#define ARITHMETIC_EVAL_PARSER_H

#include <map>
#include <memory>
#include <vector>
#include <functional>
#include "FunctionFactory.h"

namespace Arithmetic {

class Parser {
public:
  Parser();
  std::shared_ptr<Node> parse(const std::string &expr) const;

  template <typename R, typename ...Args>
  void addFunction(const std::string &name, const std::function<R(Args...)> &f) {
    m_functions[name] = std::make_shared<FunctionFactoryGenerator<std::function<R(Args...)>>>(f, name);
  };

  void addConstant(const std::string &name, double val) {
    addFunction(name, std::function<double()>{[val]()->double{return val;}});
  }

private:
  std::map<std::string, std::shared_ptr<FunctionFactory>> m_functions;
};

} // namespace Arithmetic

#endif //ARITHMETIC_EVAL_PARSER_H
