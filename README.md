# memo

[![Travis build status][travis-image]][travis-link]

``memo`` is a small C++17 library designed to allow for easy memoization of
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

This library requires a C++17-compliant compiler. Additionally, to run the
tests, you'll need [mettle][mettle].

## License

This library is licensed under the [BSD 3-Clause license](LICENSE).

[travis-image]: https://travis-ci.org/jimporter/memo.svg?branch=master
[travis-link]: https://travis-ci.org/jimporter/memo
[mettle]: https://jimporter.github.io/mettle/
