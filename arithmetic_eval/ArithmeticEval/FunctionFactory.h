#ifndef ARITHMETIC_EVAL_FUNCTIONFACTORY_H
#define ARITHMETIC_EVAL_FUNCTIONFACTORY_H

#include "Node.h"
#include <functional>
#include <memory>
#include <vector>

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
    //assert(sizeof...(Ts) == m_args.size());
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

} // namespace Arithmetic

#endif //ARITHMETIC_EVAL_FUNCTIONFACTORY_H
