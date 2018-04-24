Arithmetic Eval
===============

Or "Yet Another Shunting Yard Parser" with support for simple arithmetic
expressions, and function calling.

It is quite liberal on the accepted input (a postfix expression works
just fine).

It uses boost::variant and abuse templating to support different types
of values: from doubles to vector of boolean, including strings.

Operators are hardcoded, and part of the core, but functions can be
registered by the code using the library. It supports functions with
arbitrary signatures. For instance

```c++
parser.addFunction("sqrt", ::sqrt);
parser.addFunction<double(const std::string&)>("len", Len());
parser.addFunction<double(const std::vector<double>&)>("avg", Avg<double>());
```

It does *not* support function overloading. If you want to provide
a function that supports different parameter types (i.e. a sum for
vector of ints, floats and doubles), it can receive a raw Value
and do the template matching itself. See `test.cpp:Sum` for an example.
