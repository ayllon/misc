#ifndef ARITHMETIC_EVAL_SEPARATOR_H
#define ARITHMETIC_EVAL_SEPARATOR_H

namespace Arithmetic {

/**
 * Implements the separation logic for boost::tokenizer
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

} // namespace Arithmetic

#endif //ARITHMETIC_EVAL_SEPARATOR_H
