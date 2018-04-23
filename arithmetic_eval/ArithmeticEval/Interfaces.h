#ifndef ARITHMETIC_EVAL_INTERFACES_H
#define ARITHMETIC_EVAL_INTERFACES_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "Value.h"

namespace Arithmetic {

/// A context is a map of variable names with an associated value
typedef std::map<std::string, Value> Context;

// Forward declaration
class Node;

/**
 * @brief Interface to be implemented by code that needs to recursively visit a parsed expression.
 */
class Visitor {
public:
  /// Called when entering a non-leaf node
  virtual void enter(const Node *node) = 0;

  /// Called when exiting a non-leaf node
  virtual void exit(const Node *node) = 0;

  /// Called when on a leaf node, as a constant or a variable
  virtual void leaf(const Node *node) = 0;
};

/**
 * @brief A parsed expression is a tree formed by classes that implement this interface
 */
class Node {
public:
  /// Virtual destructor
  virtual ~Node() = default;

  /// Get a string representation of the node (typically the name or value for constants)
  virtual std::string repr() const = 0;

  /// Trigger a traversal of the tree starting at this node
  virtual void visit(Visitor *visitor) const = 0;

  /// Evaluate the tree starting at this node
  /// @param ctx  A dictionary of variable values
  virtual Value value(const Context &ctx = {}) const = 0;

  template <typename T>
  T value(const Context &ctx = {}) const {
    return Arithmetic::get<T>(value(ctx));
  }
};

/**
 * @brief Instantiate a function as a Node for the expression tree
 */
class FunctionFactory {
public:

  /// Virtual destructor
  virtual  ~FunctionFactory() = default;

  /// Return how many parameters does the function except
  virtual size_t nArgs() const = 0;

  /// Return an instance of the function
  /// @param args A vector with nArgs() nodes that are to be the children/parameters of the function
  virtual std::shared_ptr<Node> instantiate(const std::vector<std::shared_ptr<Node>> &args) const = 0;
};

} // namespace Arithmetic

#endif //ARITHMETIC_EVAL_INTERFACES_H
