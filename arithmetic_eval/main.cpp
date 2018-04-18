#include <iostream>
#include <cmath>
#include <sstream>
#include "ArithmeticEval/Parser.h"

using namespace Arithmetic;


class SqrtNode: public Node {
public:
  SqrtNode(const std::shared_ptr<Node> &a): m_a(a) {
  }

  std::string repr() const override {
    return "sqrt";
  }

  void visit(Visitor *visitor) const override {
    visitor->enter(this);
    m_a->visit(visitor);
    visitor->exit(this);
  }

  double value() const override {
    return ::sqrt(m_a->value());
  }

private:
  std::shared_ptr<Node> m_a;
};

class SqrtFactory: public FunctionFactory {
public:
  std::string getName() const override {
    return "sqrt";
  }

  size_t nArgs() const override {
    return 1;
  }

  std::shared_ptr<Node> instantiate(const std::vector<std::shared_ptr<Node>> &args) const override {
    return std::make_shared<SqrtNode>(args[0]);
  }
};

class GraphvizGenerator: public Visitor {
public:
  GraphvizGenerator() {
    m_os << "digraph G {" << std::endl;
  }

  void enter(const Node *node) override {
    m_os << '\t';
    if (!stack.empty()) {
      m_os << '"' << stack.back()->repr() << '"' << " -> ";
    }
    m_os << '"' << node->repr() << '"' << std::endl;
    stack.push_back(node);
  }

  void exit(const Node *node) override {
    stack.pop_back();
  }

  std::string str(void) const {
    return m_os.str() + "}";
  }

private:
  std::ostringstream m_os;
  std::vector<const Node*> stack;
};

int main() {
  try {
    std::string raw;
    std::cin >> raw;

    Parser parser;
    parser.registerFunction(std::make_shared<SqrtFactory>());

    auto expr = parser.parse(raw);

    GraphvizGenerator graph;
    expr->visit(&graph);
    std::cout << graph.str() << std::endl;
  }
  catch (std::exception const &e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
