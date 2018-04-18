#ifndef ARITHMETIC_EVAL_BUILTINFUNCTIONS_H
#define ARITHMETIC_EVAL_BUILTINFUNCTIONS_H

#include "FunctionFactory.h"
#include <cmath>

namespace Arithmetic {

extern std::shared_ptr<FunctionFactory>
    SqrtFunctionFactory, PowFunctionFactory, LnFunctionFactory;

} // namespace Arithmetic

#endif //ARITHMETIC_EVAL_BUILTINFUNCTIONS_H
