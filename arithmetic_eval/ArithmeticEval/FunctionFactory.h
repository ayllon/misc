#ifndef ARITHMETIC_EVAL_FUNCTIONFACTORY_H
#define ARITHMETIC_EVAL_FUNCTIONFACTORY_H

#include <functional>
#include "Parser.h"

namespace Arithmetic {

template<typename R, typename ...Args>
class FunctionNodeGenerator : public Node {
public:
  typedef std::function<R(Args...)> FuncType;

  FunctionNodeGenerator(FuncType f, const std::string &repr, const std::vector<std::shared_ptr<Node>> &args) :
      m_f(f), m_repr(repr), m_args(args) {}

  std::string repr() const override {
    return m_repr;
  }

  void visit(Visitor *visitor) const override {
    visitor->enter(this);
    for (auto arg: m_args) {
      arg->visit(visitor);
    }
    visitor->exit(this);
  }

  virtual R value(const Context &ctx) const override {
    return expand_args(ctx);
  }

private:
  FuncType m_f;
  const std::vector<std::shared_ptr<Node>> m_args;
  std::string m_repr;

  template<typename... Ts>
  typename std::enable_if<sizeof...(Args) == sizeof...(Ts), R>::type
  expand_args(const Context&, Ts &&... ts) const {
    assert(sizeof...(Ts) == m_args.size());
    return m_f(std::forward<Ts>(ts)...);
  };

  template<typename... Ts>
  typename std::enable_if<sizeof...(Args) != sizeof...(Ts), R>::type
  expand_args(const Context &ctx, Ts &&... ts) const {
    constexpr int index = sizeof...(Args) - sizeof...(Ts) - 1;
    static_assert(index >= 0, "incompatible function parameters");
    return expand_args(ctx, m_args[index]->value(ctx), std::forward<Ts>(ts)...);
  }
};


template<typename T>
class FunctionFactoryGenerator;

template<typename R, typename ...Args>
class FunctionFactoryGenerator<std::function<R(Args...)>> : public FunctionFactory {
public:
  typedef std::function<R(Args...)> FuncType;

  FunctionFactoryGenerator(FuncType f, const std::string &name): m_f(f), m_name(name) {
  }

  std::string getName() const override {
    return m_name;
  }

  size_t nArgs() const override {
    return sizeof...(Args);
  }

  std::shared_ptr<Node> instantiate(const std::vector<std::shared_ptr<Node>> &args) const override {
    return std::make_shared<FunctionNodeGenerator<R, Args...>>(m_f, m_name, args);
  }

private:
  FuncType m_f;
  std::string m_name;
};

typedef FunctionFactoryGenerator<std::function<double(double)>> UnaryFunctionFactory;
typedef FunctionFactoryGenerator<std::function<double(double, double)>> BinaryFunctionFactory;

class ConstantFunction: public Node {
public:
  ConstantFunction(double val, const std::string &name): m_val(val), m_name(name) {
  }

  std::string repr() const override {
    return m_name;
  }

  void visit(Visitor *visitor) const override {
    visitor->leaf(this);
  }

  double value(const Context &ctx) const override {
    return m_val;
  };

private:
  double m_val;
  std::string m_name;
};

class ConstantFunctionFactory: public FunctionFactory {
public:
  ConstantFunctionFactory(double val, const std::string &name): m_val(val), m_name(name) {
  }

  std::string getName() const override {
    return m_name;
  }

  size_t nArgs() const override {
    return 0;
  }

  std::shared_ptr<Node> instantiate(const std::vector<std::shared_ptr<Node>> &args) const override {
    return std::make_shared<ConstantFunction>(m_val, m_name);
  }

private:
  double m_val;
  std::string m_name;
};

} // namespace Arithmetic

#endif //ARITHMETIC_EVAL_FUNCTIONFACTORY_H
