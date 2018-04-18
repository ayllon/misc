#include "Exception.h"

namespace Arithmetic {

Exception::Exception(const std::string &msg): m_msg(msg) {}

const char *Exception::what() const noexcept {
  return m_msg.c_str();
}

} // namespace Arithmetic