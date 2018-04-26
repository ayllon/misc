#ifndef ARITHMETIC_EVAL_PARSER_H
#define ARITHMETIC_EVAL_PARSER_H

#include <map>
#include <memory>
#include <vector>
#include <functional>
#include "FunctionFactory.h"
#include "Exception.h"

namespace Arithmetic {

/// @brief Implements the parsing logic, generating a tree
class Parser {
public:

  /// Constructor
  Parser();

  /// Parse an expression
  /// @param  expr The expression to be parsed
  /// @return The root of the parsed expression
  /// @throw  If there is an error parsing the expression
  /// @note   Function instantiations are inserted into the tree in this stage
  std::unique_ptr<Node> parse(const std::string &expr) const;

  /// Register a std::function into the parser
  /// @param name The name used to call the function on an expression
  /// @param f    The functor
  template <typename R, typename ...Args>
  void addFunction(const std::string &name, const std::function<R(Args...)> &f) {
    m_functions[name] = std::make_shared<FunctionFactoryGenerator<std::function<R(Args...)>>>(name, f);
  };

  /// Register a regular function into the parser (as ::sqrt)
  /// @param name The name used to call the function on an expression
  /// @param f    The function
  template <typename Func>
  void addFunction(const std::string &name, Func &f) {
    addFunction(name, std::function<Func>(f));
  }

  /// Register a functor into the parser
  /// @param name The name used to call the function on an expression
  /// @param f    The function
  /// @note  For lambda/functor, TBase gives the signature of the function
  template <typename TBase, typename TFunctor>
  void addFunction(const std::string &name, const TFunctor &f) {
    addFunction(name, std::function<TBase>(f));
  }

  /// Convenience method to register a constant (a function with no parameters)
  /// @param name The name used to call the function on an expression
  /// @param val  The value of the constant
  void addConstant(const std::string &name, const Value &val) {
    addFunction(name, std::function<Value()>{[val]()->Value{return val;}});
  }

private:
  std::map<std::string, std::shared_ptr<FunctionFactory>> m_functions;
};

} // namespace Arithmetic

#endif //ARITHMETIC_EVAL_PARSER_H
