#ifndef ARITHMETIC_EVAL_SEPARATOR_H
#define ARITHMETIC_EVAL_SEPARATOR_H

namespace Arithmetic {

/// Implements the separation logic for boost::tokenizer
struct ArithmeticSeparator {
  /// reset does noting
  void reset() {}

  /// get next token
  template<typename InputIterator, typename Token>
  bool operator()(InputIterator &next, InputIterator end, Token &tok) {
    const std::string singleCharOperators{"/*-+()"};

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
    // Single char operators
    else if (singleCharOperators.find(*next) != std::string::npos) {
      tok.assign(next, next + 1);
      ++next;
    }
    // Just bind together the rest
    else {
      auto begin = next;
      while (next != end && !(std::isalnum(*next) || std::isspace(*next) || singleCharOperators.find(*next) != std::string::npos)) {
        ++next;
      }
      tok.assign(begin, next);
    }
    return true;
  }
};

} // namespace Arithmetic

#endif //ARITHMETIC_EVAL_SEPARATOR_H
