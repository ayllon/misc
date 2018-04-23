
#ifndef ARITHMETIC_EVAL_VALUE_H
#define ARITHMETIC_EVAL_VALUE_H

#include "Exception.h"
#include <cmath>
#include <boost/variant.hpp>

namespace Arithmetic {

typedef boost::variant<bool,
                      int32_t,
                      int64_t,
                      float,
                      double,
                      std::string,
                      std::vector<bool>,
                      std::vector<int32_t>,
                      std::vector<int64_t>,
                      std::vector<float>,
                      std::vector<double>> Value;


template <typename D>
struct CastVisitor;

template <>
struct CastVisitor<double>: public boost::static_visitor<double> {

  template <typename T>
  typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value, double>::type
  operator() (T v) const {
    return v;
  }

  template <typename T>
  typename std::enable_if<!std::is_integral<T>::value && !std::is_floating_point<T>::value, double>::type
  operator() (T v) const {
    throw Exception("Can not convert types");
  };
};

template <typename D>
D get(const Value &val) {
  return boost::get<D>(val);
};

template <>
double get(const Value &val) {
  return boost::apply_visitor(CastVisitor<double>(), val);
};

/**
 * The '+' operator is special, since it has to be able to add two doubles, but also
 * concatenate two strings
 */
struct plus {
  Value operator() (const Value &a, const Value &b) {
    if (a.type() == typeid(double) && a.type() == b.type()) {
      return Arithmetic::get<double>(a) + Arithmetic::get<double>(b);
    }
    else if (a.type() == typeid(std::string) && a.type() == b.type()) {
      return Arithmetic::get<std::string>(a) + Arithmetic::get<std::string>(b);
    }
    throw Exception("Can not evaluate both sides of the operator +");
  }
};

/**
 * Implementation of the power (^) operator
 */
struct pow {
  double operator() (double a, double b) {
    return ::pow(a, b);
  }
};

/**
 * Implementation of the module (%) operator
 */
struct mod {
  double operator() (double a, double b) {
    return ::fmod(a, b);
  }
};

};

#endif // ARITHMETIC_EVAL_VALUE_H
