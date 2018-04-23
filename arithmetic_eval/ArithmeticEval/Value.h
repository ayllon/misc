
#ifndef ARITHMETIC_EVAL_VALUE_H
#define ARITHMETIC_EVAL_VALUE_H

#include "Exception.h"
#include <cmath>
#include <boost/variant.hpp>

namespace Arithmetic {

typedef boost::variant<double, std::string> Value;

/**
 * The '+' operator is special, since it has to be able to add two doubles, but also
 * concatenate two strings
 */
struct plus {
  Value operator() (const Value &a, const Value &b) {
    if (a.type() == typeid(double) && a.type() == b.type()) {
      return boost::get<double>(a) + boost::get<double>(b);
    }
    else if (a.type() == typeid(std::string) && a.type() == b.type()) {
      return boost::get<std::string>(a) + boost::get<std::string>(b);
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
