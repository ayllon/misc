#include <boost/test/unit_test.hpp>
#include <cmath>
#include <iostream>
#include "ArithmeticEval/Exception.h"
#include "ArithmeticEval/Parser.h"
#include "ArithmeticEval/Util.h"

using namespace Arithmetic;

struct Len {
  double operator() (const std::string &str) {
    return str.size();
  }

  double operator() (const std::vector<int> &v) {
    return v.size();
  }
};

struct StrLower {
  std::string operator() (std::string str) {
    std::transform(
        str.begin(), str.end(), str.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return str;
  }
};

template <typename T>
struct Avg {
  double operator() (const std::vector<T>& v) {
    double t = 0;
    for (auto n : v) {
      t += n;
    }
    return t / v.size();
  }
};

struct Sum {
  struct SumVisitor: public boost::static_visitor<double> {

    template <typename T>
    typename std::enable_if<is_numeric<T, false>::value, double>::type
    operator() (const std::vector<T> &v) const {
      double t = 0;
      for (auto n : v) {
        t += n;
      }
      return t;
    }

    template <typename T>
    typename std::enable_if<!is_numeric<T, false>::value, double>::type
    operator() (const std::vector<T> &v) const {
      throw Exception("Unsupported vector type for sum");
    }

    template <typename T>
    typename std::enable_if<!is_vector<T>::value, double>::type
    operator() (const T&) const {
      throw Exception("Unsupported type for sum");
    };
  };

  double operator() (const Value &v) {
    return boost::apply_visitor(SumVisitor(), v);
  }
};

struct VarFixture {
  std::map<std::string, Value> variables{
      {"ID", 42},
      {"a", 10},
      {"b", 20},
      {"pi", 3.14},
      {"name", std::string{"ABCDEF"}},
      {"description", std::string{"blah bleh blih"}},
      {"vector_int", std::vector<int>{1, 2, 3}},
      {"vector_double", std::vector<double>{0.4, 5.1, 10.5}},
  };
  Parser parser;

  VarFixture() {
    parser.addFunction("sqrt", ::sqrt);
    parser.addFunction("ln", ::log);
    parser.addFunction("pow", ::pow);
    parser.addConstant("true", 1.);
    parser.addConstant("false", 0.);
    parser.addFunction<double(const std::string&)>("len", Len());
    parser.addFunction<std::string(const std::string&)>("tolower", StrLower());
    parser.addFunction<double(const std::vector<int>&)>("size", Len());
    parser.addFunction<double(const std::vector<double>&)>("avg", Avg<double>());
    parser.addFunction<double(const Value&)>("sum", Sum());
  }
};

BOOST_FIXTURE_TEST_SUITE(arithmetic, VarFixture)

BOOST_AUTO_TEST_CASE(EvalConstant) {
  auto expr = parser.parse("5");
  BOOST_CHECK_EQUAL(expr->value<double>(), 5);
}

BOOST_AUTO_TEST_CASE(EvalId) {
  auto expr = parser.parse("ID");
  BOOST_CHECK_THROW(expr->value(), Exception);
  BOOST_CHECK_THROW(expr->value({{"ANOTHER", 22}}), Exception);
  BOOST_CHECK_EQUAL(expr->value<double>(variables), 42);
}

BOOST_AUTO_TEST_CASE(SimpleExpression) {
  auto expr = parser.parse("5+2*2");
  BOOST_CHECK_EQUAL(expr->value<double>(), 9);
}

BOOST_AUTO_TEST_CASE(ParenthesisExpression) {
  BOOST_CHECK_EQUAL(parser.parse("(5+2)*2")->value<double>(), 14);
  BOOST_CHECK_EQUAL(parser.parse("((5+2)*(3+4))+1")->value<double>(), 50);
}

BOOST_AUTO_TEST_CASE(ParenthesisWithVariables) {
  BOOST_CHECK_EQUAL(parser.parse("pi+a*b")->value<double>(variables), 203.14);
}

BOOST_AUTO_TEST_CASE(Booleans) {
  BOOST_CHECK_EQUAL(parser.parse("1 && 0")->value<double>(), 0);
  BOOST_CHECK_EQUAL(parser.parse("1 && 1")->value<double>(), 1);
  BOOST_CHECK_EQUAL(parser.parse("0 && 0")->value<double>(), 0);
  BOOST_CHECK_EQUAL(parser.parse("0 || 0")->value<double>(), 0);
  BOOST_CHECK_EQUAL(parser.parse("0 || 1")->value<double>(), 1);
  BOOST_CHECK_EQUAL(parser.parse("1 || 1")->value<double>(), 1);
}

BOOST_AUTO_TEST_CASE(Comparison) {
  BOOST_CHECK_EQUAL(parser.parse("1 + 2 == 3")->value<double>(), 1);
  BOOST_CHECK_EQUAL(parser.parse("1 + 2 == 4")->value<double>(), 0);
  BOOST_CHECK_EQUAL(parser.parse("1 + 2 != 3")->value<double>(), 0);
  BOOST_CHECK_EQUAL(parser.parse("1 + 2 != 4")->value<double>(), 1);
  BOOST_CHECK_EQUAL(parser.parse("1 + 2 < 4")->value<double>(), 1);
}

BOOST_AUTO_TEST_CASE(MalformedParenthesis) {
  BOOST_CHECK_THROW(parser.parse("1 + 2)"), Exception);
  BOOST_CHECK_THROW(parser.parse(")1+2("), Exception);
  BOOST_CHECK_THROW(parser.parse("(1+2("), Exception);
}

BOOST_AUTO_TEST_CASE(BuiltInFunctions) {
  BOOST_CHECK_EQUAL(parser.parse("sqrt(4+5)")->value<double>(), 3);
  BOOST_CHECK_EQUAL(parser.parse("pow(sqrt(25), 2)")->value<double>(), 25);
  BOOST_CHECK_EQUAL(parser.parse("true || false")->value<double>(), 1.);
  BOOST_CHECK_EQUAL(parser.parse("pow(a,2)")->value<double>(variables), 100);
}

BOOST_AUTO_TEST_CASE(DecimalDelimiter) {
  BOOST_CHECK_EQUAL(parser.parse("1.4")->value<double>(), 1.4);
  BOOST_CHECK_EQUAL(parser.parse("0.4+1.1")->value<double>(), 1.5);
}

BOOST_AUTO_TEST_CASE(HexadecimalNumbers) {
  BOOST_CHECK_EQUAL(parser.parse("0xA")->value<double>(), 10);
}

BOOST_AUTO_TEST_CASE(InvalidNumbers) {
  BOOST_CHECK_THROW(parser.parse("123abc")->value(), Exception);
  BOOST_CHECK_THROW(parser.parse("0xabcyd")->value(), Exception);
}

BOOST_AUTO_TEST_CASE(Strings) {
  BOOST_CHECK_EQUAL(parser.parse("\"a string\"")->value<std::string>(), "a string");
  BOOST_CHECK_EQUAL(parser.parse("\"conca\" + \"tenate\"")->value<std::string>(), "concatenate");
  BOOST_CHECK_THROW(parser.parse("\"conca\" * \"tenate\"")->value<std::string>(), Exception);
  BOOST_CHECK_THROW(parser.parse("\"conca\" + 5")->value<std::string>(), Exception);
  BOOST_CHECK_EQUAL(parser.parse("len(\"this is a string\")")->value<double>(), 16);
}

BOOST_AUTO_TEST_CASE(StringComparison) {
  BOOST_CHECK_EQUAL(parser.parse("\"abcd\" < \"efg\"")->value<double>(), 1);
  BOOST_CHECK_EQUAL(parser.parse("\"abcd\" + \"efg\" == \"abcdefg\"")->value<double>(), 1);
}

BOOST_AUTO_TEST_CASE(StringVariables) {
  BOOST_CHECK_EQUAL(parser.parse("name")->value<std::string>(variables), "ABCDEF");
  BOOST_CHECK_EQUAL(parser.parse("len(description)")->value<double>(variables), 14);
  BOOST_CHECK_EQUAL(parser.parse("tolower(name) + \" ID\"")->value<std::string>(variables), "abcdef ID");
}

BOOST_AUTO_TEST_CASE(Vector, *boost::unit_test::tolerance(0.001)) {
  auto original_int = Arithmetic::get<std::vector<int>>(variables["vector_int"]);

  BOOST_CHECK_EQUAL(parser.parse("size(vector_int)")->value<double>(variables), 3);
  BOOST_TEST(parser.parse("vector_int")->value<std::vector<int>>(variables) == original_int, boost::test_tools::per_element());
  BOOST_TEST(parser.parse("avg(vector_double)")->value<double>(variables) == 5.333);
}

BOOST_AUTO_TEST_CASE(ValueFunction) {
  BOOST_CHECK_EQUAL(parser.parse("sum(vector_int)")->value<double>(variables), 6);
  BOOST_CHECK_EQUAL(parser.parse("sum(vector_double)")->value<double>(variables), 16);
}

BOOST_AUTO_TEST_CASE(UnaryOperators) {
  BOOST_CHECK_EQUAL(parser.parse("+1")->value<double>(), 1);
  BOOST_CHECK_EQUAL(parser.parse("-1")->value<double>(), -1);
  BOOST_CHECK_EQUAL(parser.parse("-(+1)")->value<double>(), -1);
  BOOST_CHECK_EQUAL(parser.parse("5+(-2)")->value<double>(), 3);
  BOOST_CHECK_EQUAL(parser.parse("2^-1")->value<double>(), 0.5);
}

BOOST_AUTO_TEST_SUITE_END()
