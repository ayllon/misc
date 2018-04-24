
#ifndef ARITHMETIC_EVAL_VALUE_H
#define ARITHMETIC_EVAL_VALUE_H

#include "Exception.h"
#include "Util.h"
#include <cmath>
#include <boost/variant.hpp>

namespace Arithmetic {

/// Variant type, with all the possible value types supported by the engine
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

/// Visitor for casting a Value into any other type
template <typename D>
struct CastVisitor;

/// Visitor for casting a Value into any other type.
/// Specialization for doubles
template <>
struct CastVisitor<double>: public boost::static_visitor<double> {

  /// Numeric types can be directly casted
  template <typename T>
  typename std::enable_if<is_numeric<T, true>::value, double>::type
  operator() (T v) const {
    return v;
  }


  /// Non numeric types can not be casted.
  /// @note A string could be parsed for a double value, but we are a bit more picky
  template <typename T>
  typename std::enable_if<!is_numeric<T, true>::value, double>::type
  operator() (T v) const {
    throw Exception("Can not convert types");
  };
};

/// Function to get the content stored in a Value casted to a given type.
/// Fallback to boost::get
/// @tparam D    The destination type
/// @param  val  The value to cast
/// @return      The value contained by val
/// @throw Exception if the content can not be casted
template <typename D>
D get(const Value &val) {
  return boost::get<D>(val);
};

/// Function to get the content stored in a Value casted to a given type.
/// Specialized for doubles. This is because boost::get will not transparently
/// convert between numeric types and double, but we want to be able to do so.
/// @param val   The value to cast
/// @return      The content of the value as a double
/// @throw Exception if the content can not be casted
template <>
double get<double>(const Value &val) {
  return boost::apply_visitor(CastVisitor<double>(), val);
};

/// Function to get the content stored in a Value casted to a given type.
/// Specialized for the identity. boost::get does not support identity boost::Get
/// @param val   The value to pass as-is
/// @return      Exactly what was received
template <>
const Value& get<const Value&>(const Value &val) {
  return val;
};

/// The '+' operator is special, since it has to be able to add two doubles, but also
/// concatenate two strings
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

/// Implementation of the power (^) operator
struct pow {
  double operator() (double a, double b) {
    return ::pow(a, b);
  }
};

/// Implementation of the module (%) operator
struct mod {
  double operator() (double a, double b) {
    return ::fmod(a, b);
  }
};

};

#endif // ARITHMETIC_EVAL_VALUE_H
