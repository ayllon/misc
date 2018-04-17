#ifndef _ARITHMETIC_EVAL_FUNCTIONS_H
#define _ARITHMETIC_EVAL_FUNCTIONS_H

#include "Evaluator.h"

namespace Arithmetic {

class Sqrt : public Expression {
public:
    void eval(EvalContext &) const override;

    std::string repr() const override;
};

class Pow : public Expression {
public:
    void eval(EvalContext &) const override;

    std::string repr() const override;
};

class Ln : public Expression {
public:
    void eval(EvalContext &) const override;

    std::string repr() const override;
};

std::map<std::string, std::shared_ptr<Expression>> AllFunctions();

} // namespace Eval

#endif // _ARITHMETIC_EVAL_FUNCTIONS_H
