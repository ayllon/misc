#ifndef ARITHMETIC_EVAL_NODE_H
#define ARITHMETIC_EVAL_NODE_H

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Arithmetic {

typedef std::map<std::string, double> Context;

class Node;

class Visitor {
public:
  virtual void enter(const Node *node) = 0;
  virtual void exit(const Node *node) = 0;
  virtual void leaf(const Node *node) = 0;
};

class Node {
public:
  virtual ~Node() = default;
  virtual std::string repr() const = 0;
  virtual void visit(Visitor *visitor) const = 0;
  virtual double value(const Context &ctx = {}) const = 0;
};

class FunctionFactory {
public:
  virtual  ~FunctionFactory() = default;
  virtual size_t nArgs() const = 0;
  virtual std::shared_ptr<Node> instantiate(const std::vector<std::shared_ptr<Node>> &args) const = 0;
};

} // namespace Arithmetic

#endif //ARITHMETIC_EVAL_NODE_H
