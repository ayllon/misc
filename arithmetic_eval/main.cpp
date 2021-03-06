#include <cmath>
#include <iostream>
#include <sstream>
#include <functional>
#include <boost/algorithm/string/replace.hpp>
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
    m_os << "digraph G {" << std::endl << "\tlabel=\"" << escape(label) << "\"" << std::endl;
  }

  void enter(const Node *node) override {
    m_os << "\t" << '"' << node << '"'<< " [label=\"" << escape(node->repr()) << "\"];" << std::endl;
    if (!stack.empty()) {
      m_os << "\t\"" << stack.back() << '"' << " -> \"" << node << "\"" << std::endl;
    }
    stack.push_back(node);
  }

  void exit(const Node *node) override {
    stack.pop_back();
  }

  void leaf(const Node *node) override {
    m_os << "\t" << '"' << node << '"'<< " [label=\"" << escape(node->repr()) << "\"];" << std::endl;
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

  std::string escape(const std::string &str) const {
    return boost::replace_all_copy(str, "\"", "\\\"");
  }
};

int main() {
  try {
    std::string raw;
    std::getline(std::cin, raw);

    Parser parser;

    parser.addConstant("pi", M_PI);
    parser.addConstant("e", M_E);
    parser.addFunction("sqrt", ::sqrt);
    parser.addFunction<double(const std::string&)>("len", TestStrFunc());
    parser.addFunction("ln", ::log);
    parser.addFunction<double(double)>("lambda", ([](double y)->double{return y-1;}));
    parser.addFunction<double()>("true", ([]()->double{return 1.;}));

    auto expr = parser.parse(raw);

    GraphvizGenerator graph(raw);
    expr->visit(&graph);
    std::cout << graph.str() << std::endl;

    std::cerr << expr->value<double>() << std::endl;
  }
  catch (std::exception const &e) {
    std::cerr << "Error! " << e.what() << std::endl;
  }

  return 0;
}
