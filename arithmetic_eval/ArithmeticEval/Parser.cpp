#include "Parser.h"
#include "Separator.h"
#include "Exception.h"
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <functional>
#include <cmath>


namespace Arithmetic {

template <typename T>
class Operator: public Node {
public:
  typedef std::function<T(T, T)> Functor;

  Operator(const std::string &repr, Functor f, const std::shared_ptr<Node> &a, const std::shared_ptr<Node> &b):
      m_repr(repr), m_f(f), m_a(a), m_b(b) {
  }

  std::string repr() const override {
    return m_repr;
  }

  void visit(Visitor *visitor) const override {
    visitor->enter(this);
    m_a->visit(visitor);
    m_b->visit(visitor);
    visitor->exit(this);
  }

  Value value(const Context &ctx) const override {
    return value_impl<T>(ctx);
  }

private:
  Functor m_f;
  std::shared_ptr<Node> m_a, m_b;
  std::string m_repr;

  template <typename TCast>
  typename std::enable_if<!std::is_same<TCast, Value>::value, Value>::type
  value_impl(const Context &ctx) const {
    try {
      return m_f(boost::get<TCast>(m_a->value(ctx)), boost::get<TCast>(m_b->value(ctx)));
    }
    catch (const boost::bad_get&) {
      throw Exception("Invalid types passed to the operator " + m_repr);
    }
  }

  template <typename TCast>
  typename std::enable_if<std::is_same<TCast, Value>::value, Value>::type
  value_impl(const Context &ctx) const {
    return m_f(m_a->value(ctx), m_b->value(ctx));
  }
};

class OperatorFactory: public FunctionFactory {
public:
  virtual unsigned getPrecedence() const = 0;
  virtual bool isLeftAssociative() const = 0;
};

template <typename T>
class BinaryOperatorFactory: public OperatorFactory {
public:
  BinaryOperatorFactory(typename Operator<T>::Functor f, unsigned precedence, bool leftAssociative, const std::string &repr):
      m_f(f), m_precedence(precedence), m_leftAssociative(leftAssociative), m_repr(repr) {
  }

  size_t nArgs() const override {
    return 2;
  }

  unsigned getPrecedence() const override {
    return m_precedence;
  }

  bool isLeftAssociative() const override {
    return m_leftAssociative;
  }

  std::shared_ptr<Node> instantiate(const std::vector<std::shared_ptr<Node>> &args) const override {
    assert(args.size() == 2);
    return std::make_shared<Operator<T>>(m_repr, m_f, args[0], args[1]);
  }

private:
  typename Operator<T>::Functor m_f;
  unsigned m_precedence;
  bool m_leftAssociative;
  std::string m_repr;
};


static std::map<std::string, std::shared_ptr<OperatorFactory>> knownOperators = {
    {"(", nullptr},
    {")", nullptr},
    {",", nullptr},
    {"^",  std::make_shared<BinaryOperatorFactory<double>>(pow(), 1, true, "^")},
    {"*",  std::make_shared<BinaryOperatorFactory<double>>(std::multiplies<double>(), 3, true, "*")},
    {"/",  std::make_shared<BinaryOperatorFactory<double>>(std::divides<double>(), 3, true, "/")},
    {"%",  std::make_shared<BinaryOperatorFactory<double>>(mod(), 3, true, "%")},
    {"+",  std::make_shared<BinaryOperatorFactory<Value>>(plus(), 4, true, "+")},
    {"-",  std::make_shared<BinaryOperatorFactory<double>>(std::minus<double>(), 4, true, "-")},
    {"<",  std::make_shared<BinaryOperatorFactory<Value>>(std::less<Value>(), 6, true, "<")},
    {">",  std::make_shared<BinaryOperatorFactory<Value>>(std::greater<Value>(), 6, true, ">")},
    {"<=", std::make_shared<BinaryOperatorFactory<Value>>(std::less_equal<Value>(), 6, true, "<=")},
    {">=", std::make_shared<BinaryOperatorFactory<Value>>(std::greater_equal<Value>(), 6, true, ">=")},
    {"==", std::make_shared<BinaryOperatorFactory<Value>>(std::equal_to<Value>(), 7, true, "==")},
    {"!=", std::make_shared<BinaryOperatorFactory<Value>>(std::not_equal_to<Value>(), 7, true, "!=")},
    {"&&", std::make_shared<BinaryOperatorFactory<double>>(std::logical_and<double>(), 11, true, "&&")},
    {"||", std::make_shared<BinaryOperatorFactory<double>>(std::logical_or<double>(), 12, true, "||")},
};

class Constant: public Node {
public:
  Constant(Value val): m_val(val) {
  }

  std::string repr() const override {
    return boost::lexical_cast<std::string>(m_val);
  }

  void visit(Visitor *visitor) const override {
    visitor->leaf(this);
  }

  Value value(const Context&) const override {
    return m_val;
  }

private:
  Value m_val;
};

class Variable: public Node {
public:
  Variable(const std::string &name): m_name(name) {
  }

  std::string repr() const override {
    return m_name;
  }

  void visit(Visitor *visitor) const override {
    visitor->leaf(this);
  }

  Value value(const Context &ctx) const override {
    auto var_i = ctx.find(m_name);
    if (var_i == ctx.end()) {
      throw Exception("Variable not found: " + m_name);
    }
    return var_i->second;
  }

private:
  std::string m_name;
};

Parser::Parser() {
}


static void instantiateNode(const std::shared_ptr<FunctionFactory> &factory, std::vector<std::shared_ptr<Node>> &compiled) {
  if (factory->nArgs() > compiled.size()) {
    throw Exception("Not enough parameters");
  }

  std::vector<std::shared_ptr<Node>> args(compiled.rbegin(), compiled.rbegin() + factory->nArgs());
  std::reverse(args.begin(), args.end());
  compiled.resize(compiled.size() - factory->nArgs());
  compiled.push_back(factory->instantiate(args));
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
          instantiateNode(operator_factory, compiled);
        }
        if (operators.empty()) {
          throw Exception("Missing opening parenthesis");
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

          instantiateNode(operator_factory, compiled);
        }
        operators.emplace_back(op_i->first, op_i->second);
      }
    }
    // Number
    else if (std::isdigit(token[0])) {
      size_t idx = 0;
      compiled.emplace_back(new Constant{std::stod(token, &idx)});
      if (idx != token.size()) {
        throw Exception("Invalid number: "  + token);
      }
    }
    // String
    else if (token[0] == '"') {
      compiled.emplace_back(new Constant{std::string{token.begin() +1, token.end() - 1}});
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
      throw Exception("Unknown token '" + token + "'");
    }
  }

  for (auto i = operators.rbegin(); i != operators.rend(); ++i) {
    if (i->first == "(") {
      throw Exception("Missing closing parenthesis");
    }
    instantiateNode(i->second, compiled);
  }

  if (compiled.size() != 1) {
    throw Exception("Malformed expression");
  }

  return compiled.back();
}

} // namespace Arithmetic2
