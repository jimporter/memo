#include <mettle.hpp>
#include "../include/memoizer.hpp"

using namespace mettle;
using namespace memo;

int func(int a, int b) {
  return a + b;
}

struct func_obj {
  int operator ()(int a, int b) {
    return a + b;
  }
};

struct poly_func_obj {
  int operator ()(double, double) {
    return 0;
  }
  int operator ()(int a, int b) {
    return a + b;
  }
};

suite<> mem("create memoizer", [](auto &_) {

  _.test("C function", []() {
    auto memo = memoize(func);
    expect(memo(1, 2), equal_to(3));
    expect(memo(1.0, 2.0), equal_to(3));

    int a = 1, b = 2;
    expect(memo(a, b), equal_to(3));
  });

  _.test("function object", []() {
    auto memo = memoize(func_obj{});
    expect(memo(1, 2), equal_to(3));
    expect(memo(1, 2.0), equal_to(3));

    int a = 1, b = 2;
    expect(memo(a, b), equal_to(3));
  });

  _.test("polymorphic function object", []() {
    auto memo = memoize<int(int, int)>(poly_func_obj{});
    expect(memo(1, 2), equal_to(3));
    expect(memo(2.0, 3.0), equal_to(5));

    int x = 3, y = 4;
    expect(memo(x, y), equal_to(7));
    double dx = 4, dy = 5;
    expect(memo(dx, dy), equal_to(9));
  });

  subsuite<>(_, "recursive function", [](auto &_) {
    auto lambda = [](auto &fib, size_t n) -> size_t {
      switch(n) {
      case 0:
        return 0;
      case 1:
        return 1;
      default:
        return fib(n-1) + fib(n-2);
      }
    };

    _.test("implicit signature", [lambda]() {
      auto fib = recursive_memoize(lambda);
      expect(fib(1), equal_to(1u));
      expect(fib(5), equal_to(5u));
      expect(fib(10), equal_to(55u));
    });

    _.test("result-type signature", [lambda]() {
      auto fib = recursive_memoize<size_t>(lambda);
      expect(fib(1), equal_to(1u));
      expect(fib(5), equal_to(5u));
      expect(fib(10), equal_to(55u));
    });

    _.test("explicit signature", [lambda]() {
      auto fib = recursive_memoize<size_t(size_t)>(lambda);
      expect(fib(1), equal_to(1u));
      expect(fib(5), equal_to(5u));
      expect(fib(10), equal_to(55u));
    });
  });

  _.test("number of calls", []() {
    size_t calls = 0;
    auto memo = memoize([&calls](int a, int b) {
      calls++;
      return a + b;
    });
    expect(memo(1, 2), equal_to(3));
    expect(memo(1, 2), equal_to(3));
    expect(calls, equal_to(1u));
  });

});
