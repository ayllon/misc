#include "BuiltinFunctions.h"

namespace Arithmetic {

std::shared_ptr<FunctionFactory>
    SqrtFunctionFactory{new UnaryFunctionFactory{::sqrt, "sqrt"}},
    LnFunctionFactory{new UnaryFunctionFactory{::log, "log"}},
    PowFunctionFactory{new BinaryFunctionFactory{::pow, "pow"}};

} // namespace Arithmetic