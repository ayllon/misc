#include "Parser.h"
#include "../ArithmeticEval/Separator.h"
#include "../ArithmeticEval/Evaluator.h"
#include <boost/tokenizer.hpp>
#include <functional>


namespace Arithmetic2 {

using Arithmetic::ArithmeticSeparator;
using Arithmetic::Error;

class Operator: public Node {
public:
  typedef std::function<double(double, double)> Functor;

  Operator(const std::string &repr, Functor f, std::shared_ptr<Node> &a, std::shared_ptr<Node> &b):
      m_repr(repr), m_f(f), m_a(a), m_b(b) {
  }

  std::string repr() const override {
    return "(" + m_a->repr() + " " + m_repr + " " + m_b->repr() + ")";
  }

  double value() const override {
    return m_f(m_a->value(), m_b->value());
  }

private:
  Functor m_f;
  std::shared_ptr<Node> m_a, m_b;
  std::string m_repr;
};

class OperatorFactory: public FunctionFactory {
public:
  virtual unsigned getPrecedence() const = 0;
  virtual bool isLeftAssociative() const = 0;
};

class BinaryOperatorFactory: public OperatorFactory {
public:
  BinaryOperatorFactory(Operator::Functor f, unsigned precedence, bool leftAssociative, const std::string &repr):
      m_f(f), m_precedence(precedence), m_leftAssociative(leftAssociative), m_repr(repr) {
  }

  size_t nArgs() const override {
    return 2;
  }

  std::string getName() const override {
    return m_repr;
  }

  unsigned getPrecedence() const override {
    return m_precedence;
  }

  bool isLeftAssociative() const override {
    return m_leftAssociative;
  }

  std::shared_ptr<Node> instantiate(std::vector<std::shared_ptr<Node>> &stack) const override {
    auto b = stack.back(); stack.pop_back();
    auto a = stack.back(); stack.pop_back();
    return std::make_shared<Operator>(m_repr, m_f, a, b);
  }

private:
  Operator::Functor m_f;
  unsigned m_precedence;
  bool m_leftAssociative;
  std::string m_repr;
};


static std::map<std::string, std::shared_ptr<OperatorFactory>> knownOperators = {
    {"(", nullptr},
    {")", nullptr},
    {",", nullptr},
    {"+", std::make_shared<BinaryOperatorFactory>(std::plus<double>(), 4, true, "+")},
    {"*", std::make_shared<BinaryOperatorFactory>(std::plus<double>(), 3, true, "*")},
};

class Constant: public Node {
public:
  Constant(double val): m_val(val) {
  }

  std::string repr() const override {
    return std::to_string(m_val);
  }

  double value() const override {
    return m_val;
  }

private:
  double m_val;
};

class Variable: public Node {
public:
  Variable(const std::string &name): m_name(name) {
  }

  std::string repr() const override {
    return m_name;
  }

  double value() const override {
    return 0.;
  }

private:
  std::string m_name;
};

Parser::Parser() {
}


void Parser::registerFunctions(std::initializer_list<std::shared_ptr<FunctionFactory>> &funs) {
  for (auto f : funs) {
    m_functions[f->getName()] = f;
  }
}


std::shared_ptr<Node> Parser::parse(const std::string &expr) const {
  std::vector<std::pair<std::string, std::shared_ptr<FunctionFactory>>> operators;
  std::vector<std::shared_ptr<Node>> compiled;
  boost::tokenizer<ArithmeticSeparator> tokenizer(expr, ArithmeticSeparator());

  for (auto token : tokenizer) {
    // Operator
    auto op_i = knownOperators.find(token);
    if (op_i != knownOperators.end()) {
      // Open parenthesis
      if (op_i->first == "(") {
        operators.push_back(*op_i);
      }
        // Close parenthesis and commas
      else if (op_i->first == ")" || op_i->first == ",") {
        while (!operators.empty() && operators.back().first != "(") {
          auto operator_factory = operators.back().second;
          operators.pop_back();

          if (compiled.size() < operator_factory->nArgs()) {
            throw Error("Not enough arguments for " + operator_factory->getName());
          }
          compiled.push_back(operator_factory->instantiate(compiled));
        }
        if (operators.empty()) {
          throw Error("Missing opening parenthesis");
        }
        if (op_i->first == ")") {
          operators.pop_back();
        }
      }
        // Rest
      else {
        while (!operators.empty()) {
          // If last_op is null, then it is a function
          auto last_op = dynamic_cast<OperatorFactory*>(operators.back().second.get());
          if (!(last_op == nullptr ||
                (last_op->getPrecedence() < op_i->second->getPrecedence() ||
                 last_op->getPrecedence() == op_i->second->getPrecedence() && op_i->second->isLeftAssociative())
          ) || operators.back().first == "(") {
            break;
          }

          auto operator_factory = operators.back().second;
          operators.pop_back();

          compiled.push_back(operator_factory->instantiate(compiled));
        }
        operators.emplace_back(op_i->first, op_i->second);
      }
    }
      // Number
    else if (std::isdigit(token[0])) {
      compiled.emplace_back(new Constant{std::stod(token)});
    }
      // Identifier
    else if (std::isalpha(token[0])) {
      auto fun_i = m_functions.find(token);
      if (fun_i == m_functions.end()) {
        compiled.emplace_back(new Variable{token});
      } else {
        operators.push_back(*fun_i);
      }
    }
      // Unknown!
    else {
      throw Error("Unknown token '" + token + "'");
    }
  }

  for (auto i = operators.rbegin(); i != operators.rend(); ++i) {
    if (i->first == "(") {
      throw Error("Missing closing parenthesis");
    }
    compiled.emplace_back(i->second->instantiate(compiled));
  }

  if (compiled.size() != 1) {
    throw Error("Malformed expression");
  }

  return compiled.back();
}

} // namespace Arithmetic2
