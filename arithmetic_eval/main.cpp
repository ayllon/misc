#include <iostream>
#include <boost/test/unit_test.hpp>
#include "ArithmeticEval/Evaluator.h"
#include "ArithmeticEval/Functions.h"

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
}

BOOST_AUTO_TEST_SUITE_END()
