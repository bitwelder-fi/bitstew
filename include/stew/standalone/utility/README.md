# Utilities

A can of various utilities.

* algorithm.hpp - various algorithms to execute a given function on each argument of a variadic argument pack.
* concepts.hpp - concepts used around stew library.
* function_traits.hpp - deduct function arguments, arity, type (i.e. function, method, lambda)
* lockable.hpp - brings `std::mutex` into the stew namespace and declares the `no_lock` type to mimic mutex locking in a non-thread safe setup.
* reference_counted.hpp - defines reference counted lockable object, with scope locks.
* scope_value.hpp - what it sais.
* type_traits.hpp - additional type traits to support dynamic typing. 