#ifndef ARITHMETIC_EVAL_FUNCTIONFACTORY_H
#define ARITHMETIC_EVAL_FUNCTIONFACTORY_H

#include "Interfaces.h"
#include <cassert>
#include <functional>
#include <memory>
#include <vector>

namespace Arithmetic {

/**
 * @brief Template class that generates via meta-programming a class suitable to be inserted into the parsed tree
 * @tparam R    Return type
 * @tparam Args Function arguments
 * @note This is used by FunctionFactoryGenerator. It doesn't make sense to be used directly.
 */
template<typename R, typename ...Args>
class FunctionNodeGenerator : public Node {
public:
  typedef std::function<R(Args...)> FuncType;

  /// Constructor
  /// @param repr A string representation of the function (the name)
  /// @param f    The function to be wrapped
  /// @param args A vector with the children nodes (function arguments)
  ///
  FunctionNodeGenerator(const std::string &repr, FuncType f, const std::vector<std::shared_ptr<Node>> &args) :
      m_repr(repr), m_f(f), m_args(args) {}

  /// Return the string representation (name) of this function node
  std::string repr() const override {
    return m_repr;
  }

  /// Trigger a traversal of the tree starting at this node
  void visit(Visitor *visitor) const override {
    if (m_args.empty()) {
      visitor->leaf(this);
    }
    else {
      visitor->enter(this);
      for (auto arg: m_args) {
        arg->visit(visitor);
      }
      visitor->exit(this);
    }
  }

  /// Evaluate the tree starting at this node
  /// @param ctx  A dictionary of variable values
  virtual R value(const Context &ctx) const override {
    // Argument expansion is done via meta-programming
    return expand_args(ctx);
  }

private:
  std::string m_repr;
  FuncType m_f;
  const std::vector<std::shared_ptr<Node>> m_args;

  /// Base case for the argument expansion: all vector entries have been expanded
  /// @tparam Ts  Variable number of arguments accepted.
  /// @param ts   Actual arguments. By now, all have been unrolled.
  /// @return     The value returned by the function itself
  template<typename... Ts>
  typename std::enable_if<sizeof...(Args) == sizeof...(Ts), R>::type
  expand_args(const Context&, Ts &&... ts) const {
    assert(sizeof...(Ts) == m_args.size());
    return m_f(std::forward<Ts>(ts)...);
  };

  /// Recursive case for the argument expansion
  /// @tparam Ts  Variable number of arguments accepted.
  /// @param ts   Actual arguments. Still arguments left to unroll.
  /// @return     The value returned by the function itself
  template<typename... Ts>
  typename std::enable_if<sizeof...(Args) != sizeof...(Ts), R>::type
  expand_args(const Context &ctx, Ts &&... ts) const {
    constexpr int index = sizeof...(Args) - sizeof...(Ts) - 1;
    static_assert(index >= 0, "incompatible function parameters");
    return expand_args(ctx, m_args[index]->value(ctx), std::forward<Ts>(ts)...);
  }
};


/**
 * @brief Dynamic generator of function factories.
 */
template<typename T>
class FunctionFactoryGenerator;

/**
 * @brief Dynamic generator of function factories. Specialization for a std::function.
 * @tparam R    Return type.
 * @tparam Args Argument types.
 */
template<typename R, typename ...Args>
class FunctionFactoryGenerator<std::function<R(Args...)>> : public FunctionFactory {
public:
  typedef std::function<R(Args...)> FuncType;

  /// Constructor
  /// @param name The function name
  /// @param f    The function to be wrapped
  FunctionFactoryGenerator(const std::string &name, FuncType f): m_name(name), m_f(f) {
  }

  /// Return how many parameters does the function expect
  size_t nArgs() const override {
    return sizeof...(Args);
  }

  /// Return an instance of the function
  /// @param args A vector with nArgs() nodes that are to be the children/parameters of the function
  std::shared_ptr<Node> instantiate(const std::vector<std::shared_ptr<Node>> &args) const override {
    return std::make_shared<FunctionNodeGenerator<R, Args...>>(m_name, m_f, args);
  }

private:
  std::string m_name;
  FuncType m_f;
};

} // namespace Arithmetic

#endif //ARITHMETIC_EVAL_FUNCTIONFACTORY_H
