# memo

``memo`` is a small C++ library designed to allow for easy memoization of
functions.

## Example

```c++
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

// Or a recursive function:
auto fib = memoize<size_t(size_t)>([](auto &fib, size_t n) -> size_t {
  switch(n) {
  case 0:
    return 0;
  case 1:
    return 1;
  default:
    return fib(n-1) + fib(n-2);
  }
});
```

## Requirements

Currently, this code is tested in C++14 (clang 3.4). The tests themselves
*require* C++14, but the library itself should be happy with C++11.

## License

This library is licensed under the BSD 3-Clause license.
