# memo

``memo`` is a small C++ library designed to allow for easy memoization of
functions.

## Example

```c++
int foo(int a, int b) {
  return a + b;
}

auto memo_foo = memoize(foo);
int x = memo_foo(1, 2);

auto memo_lambda = memoize((int a, int b) {
  return a + b;
});
int y = memo_lambda(1, 2);
```

## Requirements

Currently, this code is tested in C++14 (clang 3.4). The tests themselves
*require* C++14, but the library itself should be happy with C++11.

## License

This library is licensed under the BSD 3-Clause license.
