# Arguments

The base principles of dynamic typing - type gets decided on first assignment.

Missing features:

* conversions
* type register

The module provides dynamic packaging of variadic template arguments at runtime. An [Argument](./argument.hpp) object holds the value and the RTTI of the value. Invokables of the `meta` module package the C++ template arguments into a single dynamic object using [PackagedArguments](./packaged_arguments.hpp). The class provides a mechanism to package the arguments for dynamic transportation, as well as to unpack those into an invokable tuple.