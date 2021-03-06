#include "Parser.h"
#include "Separator.h"
#include "Exception.h"
#include "Util.h"
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <functional>
#include <cmath>


namespace Arithmetic {



class ValueStringRepr: public boost::static_visitor<std::string> {
public:
  template <typename T>
  typename std::enable_if<is_numeric<T, true>::value, std::string>::type
  operator() (T &val) const {
    return std::to_string(val);
  }

  template<typename T>
  typename std::enable_if<std::is_same<T, std::string>::value, std::string>::type
  operator() (const T &val) const {
    return val;
  }

  template<typename T>
  typename std::enable_if<is_vector<T>::value, std::string>::type
  operator() (const T &val) const {
    return "["  + std::to_string(val.size()) + "...]";
  }
};


template <typename T>
class UnaryOperator: public Node {
public:
  typedef std::function<T(T)> Functor;

  UnaryOperator(const std::string &repr, Functor f, std::unique_ptr<Node> a):
      m_repr(repr), m_f(f), m_a(std::move(a)) {
  }

  std::string repr() const override {
    return m_repr;
  }

  void visit(Visitor *visitor) const override {
    visitor->enter(this);
    m_a->visit(visitor);
    visitor->exit(this);
  }

  Value value(const Context &ctx) const override {
    return value_impl<T>(ctx);
  }

  bool isConstant() const override {
    return m_a->isConstant();
  }

private:
  std::string m_repr;
  Functor m_f;
  std::unique_ptr<Node> m_a;

  template <typename TCast>
  typename std::enable_if<!std::is_same<TCast, Value>::value, Value>::type
  value_impl(const Context &ctx) const {
    try {
      return m_f(Arithmetic::get<TCast>(m_a->value(ctx)));
    }
    catch (const boost::bad_get&) {
      throw Exception("Invalid types passed to the operator " + m_repr);
    }
  }

  template <typename TCast>
  typename std::enable_if<std::is_same<TCast, Value>::value, Value>::type
  value_impl(const Context &ctx) const {
    return m_f(m_a->value(ctx));
  }
};


template <typename T>
class BinaryOperator: public Node {
public:
  typedef std::function<T(T, T)> Functor;

  BinaryOperator(const std::string &repr, Functor f, std::unique_ptr<Node> a, std::unique_ptr<Node> b):
      m_repr(repr), m_f(f), m_a(std::move(a)), m_b(std::move(b)) {
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

  bool isConstant() const override {
    return m_a->isConstant() && m_b->isConstant();
  }

private:
  std::string m_repr;
  Functor m_f;
  std::unique_ptr<Node> m_a, m_b;

  template <typename TCast>
  typename std::enable_if<!std::is_same<TCast, Value>::value, Value>::type
  value_impl(const Context &ctx) const {
    try {
      return m_f(Arithmetic::get<TCast>(m_a->value(ctx)), Arithmetic::get<TCast>(m_b->value(ctx)));
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
struct identity : public std::unary_function<T, T>
{
  T operator()(const T& x) const {
    return x;
  }
};

template <typename T>
class UnaryOperatorFactory: public OperatorFactory {
public:

  UnaryOperatorFactory(typename UnaryOperator<T>::Functor f, unsigned precedence, const std::string &repr):
      m_f(f), m_precedence(precedence), m_repr(repr) {
  }

  size_t nArgs() const override {
    return 1;
  }

  unsigned getPrecedence() const override {
    return m_precedence;
  }

  bool isLeftAssociative() const override {
    return true;
  }

  std::unique_ptr<Node> instantiate(std::vector<std::unique_ptr<Node>> args) const override {
    assert(args.size() == 1);
    return std::unique_ptr<Node>{new UnaryOperator<T>{m_repr, m_f, std::move(args[0])}};
  }

private:
  typename UnaryOperator<T>::Functor m_f;
  unsigned m_precedence;
  bool m_leftAssociative;
  std::string m_repr;
};

template <typename T>
class BinaryOperatorFactory: public OperatorFactory {
public:
  BinaryOperatorFactory(typename BinaryOperator<T>::Functor f, unsigned precedence, bool leftAssociative, const std::string &repr):
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

  std::unique_ptr<Node> instantiate(std::vector<std::unique_ptr<Node>> args) const override {
    assert(args.size() == 2);
    return std::unique_ptr<Node>{new BinaryOperator<T>{m_repr, m_f, std::move(args[0]), std::move(args[1])}};
  }

private:
  typename BinaryOperator<T>::Functor m_f;
  unsigned m_precedence;
  bool m_leftAssociative;
  std::string m_repr;
};


static std::map<std::string, std::shared_ptr<OperatorFactory>> knownOperators = {
    {"(", nullptr},
    {")", nullptr},
    {",", nullptr},
    {"^",  std::make_shared<BinaryOperatorFactory<double>>(pow(), 1, false, "^")},
    {"~-", std::make_shared<UnaryOperatorFactory<double>>(std::negate<double>(), 2, "-")},
    {"~+", std::make_shared<UnaryOperatorFactory<double>>(identity<double>(), 2, "+")},
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
    return boost::apply_visitor(ValueStringRepr(), m_val);
  }

  void visit(Visitor *visitor) const override {
    visitor->leaf(this);
  }

  Value value(const Context&) const override {
    return m_val;
  }

  bool isConstant() const override {
    return true;
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

  bool isConstant() const override {
    return false;
  }

private:
  std::string m_name;
};

Parser::Parser() {
}


static void instantiateNode(const std::shared_ptr<FunctionFactory> &factory, std::vector<std::unique_ptr<Node>> &compiled) {
  if (factory->nArgs() > compiled.size()) {
    throw Exception("Not enough parameters");
  }

  typedef typename std::vector<std::unique_ptr<Node>>::reverse_iterator iter_type;

  std::vector<std::unique_ptr<Node>> args(
      std::move_iterator<iter_type>(compiled.rbegin()),
      std::move_iterator<iter_type>(compiled.rbegin() + factory->nArgs())
  );
  std::reverse(args.begin(), args.end());
  compiled.resize(compiled.size() - factory->nArgs());

  auto newNode = factory->instantiate(std::move(args));

  // Fold the node if is constant
  if (newNode->isConstant()) {
    newNode.reset(new Constant(newNode->value({})));
  }

  compiled.emplace_back(std::move(newNode));
}


std::unique_ptr<Node> Parser::parse(const std::string &expr) const {
  std::vector<std::pair<std::string, std::shared_ptr<FunctionFactory>>> operators;
  std::vector<std::unique_ptr<Node>> compiled;
  boost::tokenizer<ArithmeticSeparator> tokenizer(expr, ArithmeticSeparator());

  std::string prevToken;

  for (auto token : tokenizer) {
    auto op_i = knownOperators.find(token);
    auto op_prev = knownOperators.find(prevToken);

    // Hack for unary + and -
    // If they are at the beginning, just after a '(', or after another operator
    if ((token == "-" || token == "+") && (prevToken.empty() || (op_prev != knownOperators.end() && op_prev->first != ")"))) {
      token = "~" + token;
      op_i = knownOperators.find(token);
      operators.emplace_back(*op_i);
    }
    // Regular operator
    else if (op_i != knownOperators.end()) {
      // Open parenthesis
      if (op_i->first == "(") {
        operators.emplace_back(*op_i);
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
                 (last_op->getPrecedence() == op_i->second->getPrecedence() && op_i->second->isLeftAssociative()))
          ) || operators.back().first == "(" || op_i->first[0] == '~') {
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

    prevToken = token;
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

  return std::move(compiled.back());
}

} // namespace Arithmetic2
