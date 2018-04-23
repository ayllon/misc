#ifndef ARITHMETIC_EVAL_SEPARATOR_H
#define ARITHMETIC_EVAL_SEPARATOR_H

namespace Arithmetic {

/**
 * Implements the separation logic for boost::tokenizer
 */
struct ArithmeticSeparator {
  void reset() {}

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

    // Numbers
    if (std::isdigit(*next)) {
      auto begin = next;
      while (next != end && (std::isxdigit(*next) || *next == '.' || *next == 'x')) {
        ++next;
      }
      tok.assign(begin, next);
    }
    // Strings
    else if (*next == '"') {
      auto begin = next;
      do {
        ++next;
      } while (next != end && *next != '"');
      if (next != end) ++next; // include last quote
      tok.assign(begin, next);
    }
    // Identifiers
    else if (std::isalpha(*next) || *next == '_') {
      auto begin = next;
      while (next != end && (std::isalnum(*next) || *next == '_')) {
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
