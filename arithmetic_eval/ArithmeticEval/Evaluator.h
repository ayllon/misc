
#ifndef _ARITHMETIC_EVAL_H
#define _ARITHMETIC_EVAL_H

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Arithmetic {

class Error : public std::exception {
private:
    std::string m_what;
public:
    Error(std::string const &msg);

    const char *what() const noexcept override;
};


struct EvalContext {
    std::map<std::string, double> const &variables;
    std::vector<double> stack;

    void push(double v) {
      stack.push_back(v);
    }

    double pop(void) {
      if (stack.empty()) {
        throw Error("Not enough values on the stack, probably malformed expression");
      }

      double v = stack.back();
      stack.pop_back();
      return v;
    }
};


class Expression {
public:
    virtual ~Expression() = default;

    virtual void eval(EvalContext &) const = 0;

    virtual std::string repr() const = 0;
};

class Operator : public Expression {
public:
    virtual unsigned getPrecedence() const = 0;

    virtual bool isLeftAssociative() const = 0;
};


class Evaluator {
public:
    Evaluator(std::string const &expr,
              std::map<std::string, std::shared_ptr<Expression>> const &functions = {});

    double operator()(std::map<std::string, double> const &variables = {}) const;

    std::string repr() const;

private:
    std::vector<std::shared_ptr<Expression>> compiled;
    std::map<std::string, std::shared_ptr<Operator>> knownOperators;
    std::map<std::string, std::shared_ptr<Expression>> knownFunctions;

    void parse(std::string const &);
};

} // namespace Arithmetic

#endif // _ARITHMETIC_EVAL_H
