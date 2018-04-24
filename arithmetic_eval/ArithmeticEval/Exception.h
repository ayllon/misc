#ifndef ARITHMETIC_EVAL_EXCEPTION_H
#define ARITHMETIC_EVAL_EXCEPTION_H

#include <exception>
#include <string>

namespace Arithmetic {

/// Exception thrown on error
class Exception: public std::exception {
public:

  /// Constructor
  /// @param msg The error message
  Exception(const std::string &msg);

  /// Return the error message
  const char *what() const noexcept override;

private:
  std::string m_msg;
};

} // namespace Arithmetic

#endif //ARITHMETIC_EVAL_EXCEPTION_H
