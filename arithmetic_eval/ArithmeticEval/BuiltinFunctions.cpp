#include "BuiltinFunctions.h"

namespace Arithmetic {

std::shared_ptr<FunctionFactory>
    SqrtFunctionFactory{new UnaryFunctionFactory{::sqrt, "sqrt"}},
    LnFunctionFactory{new UnaryFunctionFactory{::log, "log"}},
    PowFunctionFactory{new BinaryFunctionFactory{::pow, "pow"}},
    TrueFunctionFactory{new ConstantFunctionFactory{1., "true"}},
    FalseFunctionFactory{new ConstantFunctionFactory{0., "false"}};

} // namespace Arithmetic