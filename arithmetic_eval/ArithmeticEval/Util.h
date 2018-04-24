#ifndef ARITHMETIC_EVAL_UTIL_H
#define ARITHMETIC_EVAL_UTIL_H

#include <vector>

namespace Arithmetic {

/// Trait to check for a vector of any type. Base case.
template <typename T>
struct is_vector {
  static const bool value = false;
};

/// Trait to check for a vector of any type. Matching case.
/// @tparam T The type contained by the vector
template <typename T>
struct is_vector<std::vector<T>> {
  static const bool value = true;
  typedef T contained_type;
};

/// Trait to check for a numeric type: floating point and integers.
/// @tparam T    The type to check
/// @tparam accept_bool  If true, bool will be considered as a numeric.
template <typename T, bool accept_bool = false>
struct is_numeric {
  static const bool value =
      std::is_floating_point<T>::value ||
      (std::is_integral<T>::value && (!std::is_same<T, bool>::value || accept_bool));
};

} // namespace Arithmetic

#endif //ARITHMETIC_EVAL_UTIL_H
