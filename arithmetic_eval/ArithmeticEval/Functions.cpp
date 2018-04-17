#include <cmath>
#include "Functions.h"

namespace Arithmetic {

void Sqrt::eval(EvalContext &context) const {
  context.push(std::sqrt(context.pop()));
}

std::string Sqrt::repr() const {
  return "Sqrt<1>";
}

void Pow::eval(EvalContext &context) const {
  double b = context.pop();
  double a = context.pop();
  context.push(std::pow(a, b));
}

std::string Pow::repr() const {
  return "Pow<2>";
}

void Ln::eval(EvalContext &context) const {
  context.push(std::log(context.pop()));
}

std::string Ln::repr() const {
  return "Ln<1>";
}

void True::eval(Arithmetic::EvalContext &context) const {
  context.push(1);
}

std::string True::repr() const {
  return "true";
}

void False::eval(Arithmetic::EvalContext &context) const {
  context.push(0);
}

std::string False::repr() const {
  return "false";
}

std::map<std::string, std::shared_ptr<Expression>> AllFunctions() {
  return {
    {"sqrt", std::make_shared<Sqrt>()},
    {"pow", std::make_shared<Pow>()},
    {"ln", std::make_shared<Ln>()},
    {"true", std::make_shared<True>()},
    {"false", std::make_shared<False>()}
  };
};

} // namespace Arithmetic
