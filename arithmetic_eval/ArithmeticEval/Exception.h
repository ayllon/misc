#ifndef ARITHMETIC_EVAL_EXCEPTION_H
#define ARITHMETIC_EVAL_EXCEPTION_H

#include <exception>
#include <string>

namespace Arithmetic {

class Exception: public std::exception {
public:
  Exception(const std::string &msg);
  const char *what() const noexcept override;

private:
  std::string m_msg;
};

} // namespace Arithmetic

#endif //ARITHMETIC_EVAL_EXCEPTION_H
