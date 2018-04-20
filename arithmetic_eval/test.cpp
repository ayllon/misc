#include <boost/test/unit_test.hpp>
#include <cmath>
#include "ArithmeticEval/Exception.h"
#include "ArithmeticEval/Parser.h"

using namespace Arithmetic;

struct VarFixture {
  std::map<std::string, double> variables{
      {"ID", 42},
      {"a", 10},
      {"b", 20},
      {"pi", 3.14}
  };
  Parser parser;

  VarFixture() {
    parser.addFunction("sqrt", std::function<decltype(::sqrt)>{::sqrt});
    parser.addFunction("ln", std::function<decltype(::log)>{::log});
    parser.addFunction("pow", std::function<decltype(::pow)>{::pow});
    parser.addConstant("true", 1.);
    parser.addConstant("false", 0.);
  }
};

BOOST_FIXTURE_TEST_SUITE(arithmetic, VarFixture)

BOOST_AUTO_TEST_CASE(EvalConstant) {
  auto expr = parser.parse("5");
  BOOST_CHECK_EQUAL(expr->value(), 5);
}

BOOST_AUTO_TEST_CASE(EvalId) {
  auto expr = parser.parse("ID");
  BOOST_CHECK_THROW(expr->value(), Exception);
  BOOST_CHECK_THROW(expr->value({{"ANOTHER", 22}}), Exception);
  BOOST_CHECK_EQUAL(expr->value(variables), 42);
}

BOOST_AUTO_TEST_CASE(SimpleExpression) {
  auto expr = parser.parse("5+2*2");
  BOOST_CHECK_EQUAL(expr->value(), 9);
}

BOOST_AUTO_TEST_CASE(ParenthesisExpression) {
  BOOST_CHECK_EQUAL(parser.parse("(5+2)*2")->value(), 14);
  BOOST_CHECK_EQUAL(parser.parse("((5+2)*(3+4))+1")->value(), 50);
}

BOOST_AUTO_TEST_CASE(ParenthesisWithVariables) {
  BOOST_CHECK_EQUAL(parser.parse("pi+a*b")->value(variables), 203.14);
}

BOOST_AUTO_TEST_CASE(Booleans) {
  BOOST_CHECK_EQUAL(parser.parse("1 && 0")->value(), 0);
  BOOST_CHECK_EQUAL(parser.parse("1 && 1")->value(), 1);
  BOOST_CHECK_EQUAL(parser.parse("0 && 0")->value(), 0);
  BOOST_CHECK_EQUAL(parser.parse("0 || 0")->value(), 0);
  BOOST_CHECK_EQUAL(parser.parse("0 || 1")->value(), 1);
  BOOST_CHECK_EQUAL(parser.parse("1 || 1")->value(), 1);
}

BOOST_AUTO_TEST_CASE(Comparison) {
  BOOST_CHECK_EQUAL(parser.parse("1 + 2 == 3")->value(), 1);
  BOOST_CHECK_EQUAL(parser.parse("1 + 2 == 4")->value(), 0);
  BOOST_CHECK_EQUAL(parser.parse("1 + 2 != 3")->value(), 0);
  BOOST_CHECK_EQUAL(parser.parse("1 + 2 != 4")->value(), 1);
  BOOST_CHECK_EQUAL(parser.parse("1 + 2 < 4")->value(), 1);
}

BOOST_AUTO_TEST_CASE(MalformedParenthesis) {
  BOOST_CHECK_THROW(parser.parse("1 + 2)"), Exception);
  BOOST_CHECK_THROW(parser.parse(")1+2("), Exception);
  BOOST_CHECK_THROW(parser.parse("(1+2("), Exception);
}

BOOST_AUTO_TEST_CASE(BuiltInFunctions) {
  BOOST_CHECK_EQUAL(parser.parse("sqrt(4+5)")->value(), 3);
  BOOST_CHECK_EQUAL(parser.parse("pow(sqrt(25), 2)")->value(), 25);
  BOOST_CHECK_EQUAL(parser.parse("true || false")->value(), 1.);
  BOOST_CHECK_EQUAL(parser.parse("pow(a,2)")->value(variables), 100);
}

BOOST_AUTO_TEST_CASE(DecimalDelimiter) {
  BOOST_CHECK_EQUAL(parser.parse("1.4")->value(), 1.4);
  BOOST_CHECK_EQUAL(parser.parse("0.4+1.1")->value(), 1.5);
}

BOOST_AUTO_TEST_CASE(HexadecimalNumbers) {
  BOOST_CHECK_EQUAL(parser.parse("0xA")->value(), 10);
}

BOOST_AUTO_TEST_CASE(InvalidNumbers) {
  BOOST_CHECK_THROW(parser.parse("123abc")->value(), Exception);
  BOOST_CHECK_THROW(parser.parse("0xabcyd")->value(), Exception);
}

BOOST_AUTO_TEST_SUITE_END()
