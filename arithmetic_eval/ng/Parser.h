#ifndef ARITHMETIC_EVAL_PARSER_H
#define ARITHMETIC_EVAL_PARSER_H

#include <map>
#include <memory>
#include <vector>
#include <boost/variant.hpp>

namespace Arithmetic2 {

class Node {
public:
  virtual ~Node() = default;
  virtual std::string repr() const = 0;
  virtual double value() const = 0;
};

class FunctionFactory {
public:
  virtual  ~FunctionFactory() = default;
  virtual std::string getName() const = 0;
  virtual size_t nArgs() const = 0;
  virtual std::shared_ptr<Node> instantiate(const std::vector<std::shared_ptr<Node>> &args) const = 0;
};

class Parser {
public:
  Parser();
  void registerFunction(const std::shared_ptr<FunctionFactory> &f);
  std::shared_ptr<Node> parse(const std::string &expr) const;

private:
  std::map<std::string, std::shared_ptr<FunctionFactory>> m_functions;
};

} // namespace Arithmetic

#endif //ARITHMETIC_EVAL_PARSER_H