#include <cctype>
#include <boost/tokenizer.hpp>
#include <sstream>
#include <cmath>
#include <functional>
#include <iostream>

#include "Evaluator.h"

namespace Arithmetic {

/**
 * Implements the separation logic for the tokenizer
 */
struct ArithmeticSeparator {
  void reset() {}

  template<typename Iterator>
  Iterator findEndOfOperator(Iterator start, Iterator end) {
    return start + 1;
  }

  template<typename InputIterator, typename Token>
  bool operator()(InputIterator &next, InputIterator end, Token &tok) {
    tok.clear();

    // Skip spaces
    while (std::isspace(*next)) {
      ++next;
    }

    if (next == end) {
      return false;
    }

    // Numbers and names
    if (std::isalnum(*next)) {
      auto begin = next;
      while (next != end && std::isalnum(*next)) {
        ++next;
      }
      tok.assign(begin, next);
    }
    // Parenthesis are individual
    else if (*next == '(' || *next == ')') {
      tok.assign(next, next + 1);
      ++next;
    }
    // Just bind together operator characters, except parenthesis
    else {
      auto begin = next;
      while (next != end && !(std::isalnum(*next) || std::isspace(*next) || *next == '(' || *next == ')')) {
        ++next;
      }
      tok.assign(begin, next);
    }
    return true;
  }
};


Error::Error(std::string const &msg) : m_what(msg) {}


const char *Error::what() const noexcept {
  return m_what.c_str();
}


class ConstantExpression : public Expression {
private:
    double m_val;
public:
    ConstantExpression(std::string const &expr) {
      m_val = std::stod(expr);
    }

    void eval(EvalContext &context) const override {
      context.push(m_val);
    }

    std::string repr() const override {
      return std::to_string(m_val);
    }
};


class VariableExpression : public Expression {
private:
    std::string m_key;

public:
    VariableExpression(std::string const &expr) : m_key(expr) {
    }

    void eval(EvalContext &context) const override {
      auto i = context.variables.find(m_key);
      if (i == context.variables.end()) {
        throw Error("Variable not defined: " + m_key);
      }
      context.push(i->second);
    }

    std::string repr() const override {
      return m_key;
    }
};


void Evaluator::parse(std::string const &expr) {
  std::vector<std::pair<std::string, std::shared_ptr<Expression>>> operators;
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
          compiled.push_back(operators.back().second);
          operators.pop_back();
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
          auto last_op = dynamic_cast<Operator *>(operators.back().second.get());
          if (!(last_op == nullptr ||
                (last_op->getPrecedence() < op_i->second->getPrecedence() ||
                 last_op->getPrecedence() == op_i->second->getPrecedence() && op_i->second->isLeftAssociative())
                ) || operators.back().first == "(") {
            break;
          }

          compiled.push_back(operators.back().second);
          operators.pop_back();
        }
        operators.push_back(*op_i);
      }
    }
    // Number
    else if (std::isdigit(token[0])) {
      compiled.emplace_back(new ConstantExpression{token});
    }
    // Identifier
    else if (std::isalpha(token[0])) {
      auto fun_i = knownFunctions.find(token);
      if (fun_i == knownFunctions.end()) {
        compiled.emplace_back(new VariableExpression{token});
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
    compiled.emplace_back(i->second);
  }
}


class BinaryOperatorAdapter : public Operator {
public:
    typedef std::function<double(double, double)> Functor;

    BinaryOperatorAdapter(Functor f, unsigned precedence, bool leftAssociative, const char *repr) :
        m_f(f), m_precedence(precedence), m_leftAssociative(leftAssociative), m_repr(repr) {
    }

    unsigned getPrecedence() const override {
      return m_precedence;
    }

    bool isLeftAssociative() const override {
      return m_leftAssociative;
    }

    void eval(EvalContext &context) const override {
      typedef decltype(context.pop()) value_type;
      value_type b = context.pop();
      value_type a = context.pop();
      context.push(m_f(a, b));
    }

    std::string repr() const override {
      return m_repr;
    }

private:
    Functor m_f;
    unsigned m_precedence;
    bool m_leftAssociative;
    const char *m_repr;
};

class UnaryOperatorAdapter : public Operator {
public:
    typedef std::function<double(double)> Functor;

    UnaryOperatorAdapter(Functor f, unsigned precedence, bool leftAssociative, const char *repr) :
        m_f(f), m_precedence(precedence), m_leftAssociative(leftAssociative), m_repr(repr) {
    }

    unsigned getPrecedence() const override {
      return m_precedence;
    }

    bool isLeftAssociative() const override {
      return m_leftAssociative;
    }

    void eval(EvalContext &context) const override {
      context.push(m_f(context.pop()));
    }

    std::string repr() const override {
      return m_repr;
    }

private:
    Functor m_f;
    unsigned m_precedence;
    bool m_leftAssociative;
    const char *m_repr;
};


Evaluator::Evaluator(std::string const &expr,
                     std::map<std::string, std::shared_ptr<Expression>> const &functions)
    : knownOperators{
    {"(",  nullptr},
    {")",  nullptr},
    {",",  nullptr},
    {"!",  std::make_shared<UnaryOperatorAdapter>(std::logical_not<double>(), 2, false, "!")},
    {"*",  std::make_shared<BinaryOperatorAdapter>(std::multiplies<double>(), 3, true, "*")},
    {"/",  std::make_shared<BinaryOperatorAdapter>(std::divides<double>(), 3, true, "*")},
    {"%",  std::make_shared<BinaryOperatorAdapter>(::fmod, 3, true, "%")},
    {"+",  std::make_shared<BinaryOperatorAdapter>(std::plus<double>(), 4, true, "+")},
    {"-",  std::make_shared<BinaryOperatorAdapter>(std::minus<double>(), 4, true, "-")},
    {"<",  std::make_shared<BinaryOperatorAdapter>(std::less<double>(), 6, true, "<")},
    {">",  std::make_shared<BinaryOperatorAdapter>(std::greater<double>(), 6, true, ">")},
    {"<=", std::make_shared<BinaryOperatorAdapter>(std::less_equal<double>(), 6, true, "<=")},
    {">=", std::make_shared<BinaryOperatorAdapter>(std::greater_equal<double>(), 6, true, ">=")},
    {"==", std::make_shared<BinaryOperatorAdapter>(std::equal_to<double>(), 7, true, "==")},
    {"!=", std::make_shared<BinaryOperatorAdapter>(std::not_equal_to<double>(), 7, true, "!=")},
    {"&&", std::make_shared<BinaryOperatorAdapter>(std::logical_and<double>(), 11, true, "&&")},
    {"||", std::make_shared<BinaryOperatorAdapter>(std::logical_or<double>(), 12, true, "||")},
}, knownFunctions{functions} {
  parse(expr);
}


double Evaluator::operator()(std::map<std::string, double> const &variables) const {
  EvalContext context{variables};
  for (auto expr : compiled) {
    expr->eval(context);
  }
  return context.stack.back();
}


std::string Evaluator::repr() const {
  std::ostringstream str;
  for (auto expr : compiled) {
    str << expr->repr() << " ";
  }
  return str.str();
}

} // Arithmetic
