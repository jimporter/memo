#include <mettle.hpp>

#include "../include/memoizer.hpp"

using namespace mettle;
using namespace memo;

int foo(int a, int b) {
  return a + b;
}

suite<> mem("test memoizer", [](auto &_) {

  _.test("basic", []() {
    auto memo = memoize(foo);
    expect(memo(1, 2), equal_to(3));
  });

  _.test("with variables", []() {
    int a = 1, b = 2;
    auto memo = memoize(foo);
    expect(memo(a, b), equal_to(3));
  });


  _.test("number of calls", []() {
    size_t calls = 0;
    auto memo = memoize([&calls](int a, int b) {
      calls++;
      return a + b;
    });
    expect(memo(1, 2), equal_to(3));
    expect(memo(1, 2), equal_to(3));
    expect(calls, equal_to<size_t>(1));
  });

  _.test("recursive memoizer", []() {
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
    expect(fib(1), equal_to(1u));
    expect(fib(5), equal_to(5u));
    expect(fib(10), equal_to(55u));
  });

});
