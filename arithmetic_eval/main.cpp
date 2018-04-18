#include <iostream>
#include <boost/test/unit_test.hpp>
#include <cmath>
#include "ArithmeticEval/Evaluator.h"
#include "ArithmeticEval/Functions.h"

#include "ng/Parser.h"

using namespace Arithmetic;

struct VarFixture {
  std::map<std::string, double> variables{
      {"ID", 42},
      {"a", 10},
      {"b", 20},
      {"pi", 3.14}
  };
  std::map<std::string, std::shared_ptr<Expression>> functions = AllFunctions();
};

BOOST_FIXTURE_TEST_SUITE(arithmetic, VarFixture)

BOOST_AUTO_TEST_CASE(EvalConstant) {
  Evaluator evaluator("5");
  BOOST_CHECK_EQUAL(evaluator(), 5);
}

BOOST_AUTO_TEST_CASE(EvalId) {
  Evaluator evaluator("ID");
  BOOST_CHECK_THROW(evaluator(), Error);
  BOOST_CHECK_THROW(evaluator({{"ANOTHER", 22}}), Error);
  BOOST_CHECK_EQUAL(evaluator(variables), 42);
}

BOOST_AUTO_TEST_CASE(SimpleExpression) {
  Evaluator evaluator("5+2*2");
  BOOST_CHECK_EQUAL(evaluator(), 9);
}

BOOST_AUTO_TEST_CASE(ParenthesisExpression) {
  BOOST_CHECK_EQUAL(Evaluator("(5+2)*2")(), 14);
  BOOST_CHECK_EQUAL(Evaluator("((5+2)*(3+4))+1")(), 50);
}

BOOST_AUTO_TEST_CASE(ParenthesisWithVariables) {
  BOOST_CHECK_EQUAL(Evaluator("pi+a*b")(variables), 203.14);
}

BOOST_AUTO_TEST_CASE(Booleans) {
  BOOST_CHECK_EQUAL(Evaluator("1 && 0")(), 0);
  BOOST_CHECK_EQUAL(Evaluator("1 && 1")(), 1);
  BOOST_CHECK_EQUAL(Evaluator("0 && 0")(), 0);
  BOOST_CHECK_EQUAL(Evaluator("0 || 0")(), 0);
  BOOST_CHECK_EQUAL(Evaluator("0 || 1")(), 1);
  BOOST_CHECK_EQUAL(Evaluator("1 || 1")(), 1);
}

BOOST_AUTO_TEST_CASE(Comparison) {
  BOOST_CHECK_EQUAL(Evaluator("1 + 2 == 3")(), 1);
  BOOST_CHECK_EQUAL(Evaluator("1 + 2 == 4")(), 0);
  BOOST_CHECK_EQUAL(Evaluator("1 + 2 != 3")(), 0);
  BOOST_CHECK_EQUAL(Evaluator("1 + 2 != 4")(), 1);
  BOOST_CHECK_EQUAL(Evaluator("1 + 2 < 4")(), 1);
}

BOOST_AUTO_TEST_CASE(MalformedParenthesis) {
  BOOST_CHECK_THROW(Evaluator("1 + 2)"), Error);
  BOOST_CHECK_THROW(Evaluator(")1+2("), Error);
  BOOST_CHECK_THROW(Evaluator("(1+2("), Error);
}

BOOST_AUTO_TEST_CASE(BuiltInFunctions) {
  BOOST_CHECK_EQUAL(Evaluator("sqrt(4+5)", functions)(), 3);
  BOOST_CHECK_EQUAL(Evaluator("pow(sqrt(25), 2)", functions)(), 25);
  BOOST_CHECK_EQUAL(Evaluator("true || false", functions)(), 1.);
  BOOST_CHECK_EQUAL(Evaluator("pow(a,2)", functions)(variables), 100);
}

BOOST_AUTO_TEST_SUITE_END()

class SqrtNode: public Arithmetic2::Node {
public:
  SqrtNode(const std::shared_ptr<Node> &a): m_a(a) {
  }

  std::string repr() const override {
    return "sqrt(" + m_a->repr() + ")";
  }

  double value() const override {
    return ::sqrt(m_a->value());
  }

private:
  std::shared_ptr<Node> m_a;
};

class SqrtFactory: public Arithmetic2::FunctionFactory {
public:
  std::string getName() const override {
    return "sqrt";
  }

  size_t nArgs() const override {
    return 1;
  }

  std::shared_ptr<Arithmetic2::Node> instantiate(const std::vector<std::shared_ptr<Arithmetic2::Node>> &args) const override {
    return std::make_shared<SqrtNode>(args[0]);
  }
};

int main() {
  try {
    Arithmetic2::Parser parser;
    parser.registerFunction(std::make_shared<SqrtFactory>());
    auto expr = parser.parse("5 + 6 * (2 + 1)");

    std::cout << expr->repr() << std::endl;

    expr = parser.parse("sqrt(5, 3) + 6 - 3");
    std::cout << expr->repr() << std::endl;
  }
  catch (std::exception const &e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}