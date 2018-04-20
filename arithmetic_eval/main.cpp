#include <iostream>
#include <sstream>
#include <functional>
#include "ArithmeticEval/Parser.h"
#include "ArithmeticEval/BuiltinFunctions.h"

using namespace Arithmetic;


class GraphvizGenerator: public Visitor {
public:
  GraphvizGenerator(const std::string &label) {
    m_os << "digraph G {" << std::endl << "\tlabel=\"" << label << "\"" << std::endl;
  }

  void enter(const Node *node) override {
    m_os << "\t" << '"' << node << '"'<< " [label=\"" << node->repr() << "\"];" << std::endl;
    if (!stack.empty()) {
      m_os << "\t\"" << stack.back() << '"' << " -> \"" << node << "\"" << std::endl;
    }
    stack.push_back(node);
  }

  void exit(const Node *node) override {
    stack.pop_back();
  }

  void leaf(const Node *node) override {
    m_os << "\t" << '"' << node << '"'<< " [label=\"" << node->repr() << "\"];" << std::endl;
    if (!stack.empty()) {
      m_os << "\t\"" << stack.back() << '"' << " -> \"" << node << "\"" << std::endl;
    }
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
    std::getline(std::cin, raw);
    //std::cin >> raw;

    Parser parser;
    parser.registerFunction(SqrtFunctionFactory);
    parser.registerFunction(LnFunctionFactory);
    parser.registerFunction(PowFunctionFactory);

    auto expr = parser.parse(raw);

    GraphvizGenerator graph(raw);
    expr->visit(&graph);
    std::cout << graph.str() << std::endl;
  }
  catch (std::exception const &e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
