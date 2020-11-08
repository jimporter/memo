# memo

[![Build status][ci-image]][ci-link]

``memo`` is a small C++17 library designed to allow for easy memoization of
functions.

## Example

```c++
#include <memo.hpp>

// Use a regular function:
int foo(int a, int b) {
  return a + b;
}
auto memo_foo = memoize(foo);
int x = memo_foo(1, 2);

// Or a lambda:
auto memo_lambda = memoize([](int a, int b) {
  return a + b;
});
int y = memo_lambda(1, 2);

// Or a polymorphic function object:
struct bar {
  int operator ()(int a, int b);
  float operator ()(float a, float b);
};
auto memo_bar = memoize<int(int, int)>(bar{});

// Or a recursive function:
auto fib = recursive_memoize([](auto &fib, size_t n) -> size_t {
  switch(n) {
  case 0:
    return 0;
  case 1:
    return 1;
  default:
    return fib(n-1) + fib(n-2);
  }
});

// You can also specify the return type of your recursive function if the above
// fails:
auto fib2 = recursive_memoize<size_t>(...);

// Or even the full signature:
auto fib3 = recursive_memoize<size_t(size_t)>(...);
```

## Requirements

This library requires a C++17-compliant compiler. Additionally, to run the
tests, you'll need [mettle][mettle].

## License

This library is licensed under the [BSD 3-Clause license](LICENSE).

[ci-image]: https://github.com/jimporter/memo/workflows/build/badge.svg
[ci-link]: https://github.com/jimporter/memo/actions?query=branch%3Amaster+workflow%3Abuild
[mettle]: https://jimporter.github.io/mettle/
