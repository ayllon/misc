#include <cmath>
#include <iostream>
#include <sstream>
#include <functional>
#include "ArithmeticEval/Parser.h"

using namespace Arithmetic;

class TestStrFunc {
public:
  double operator() (const std::string &str) {
    return str.size();
  }
};

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
    std::string raw("sqrt(len(\"abc\"))");
    //std::getline(std::cin, raw);

    Parser parser;

    parser.addFunction("sqrt", ::sqrt);
    parser.addFunction<double(const std::string&)>("len", TestStrFunc());
//    parser.addFunction("ln", ::log);
//    parser.addFunction("pow", ::pow);
//    parser.addFunction<double(double,double,double)>("test", TestFunctor());
//    parser.addFunction<double(double)>("lambda", ([](double y)->double{return y-1;}));
//    parser.addFunction<double()>("true", ([]()->double{return 1.;}));

    auto expr = parser.parse(raw);

    GraphvizGenerator graph(raw);
    expr->visit(&graph);
    std::cout << graph.str() << std::endl;

    std::cerr << expr->value() << std::endl;
  }
  catch (std::exception const &e) {
    std::cerr << "Error! " << e.what() << std::endl;
  }

  return 0;
}
