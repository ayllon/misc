
#ifndef ARITHMETIC_EVAL_VALUE_H
#define ARITHMETIC_EVAL_VALUE_H

#include <boost/variant.hpp>

namespace Arithmetic {

typedef boost::variant<int64_t, double, std::string> Value;

struct multiplies {
  Value operator() (const Value &a, const Value &b) {
    return 0.;
  }
};

};

#endif // ARITHMETIC_EVAL_VALUE_H
