#include <mettle.hpp>
#include "../include/memoizer.hpp"

using namespace mettle;
using namespace memo;

struct copy_tracker {
  copy_tracker() : instances(0), copies(0), moves(0) {}
  size_t instances, copies, moves;
};

struct my_type {
  my_type(int value, copy_tracker &tracker) : value(value), tracker(tracker) {
    tracker.instances++;
  }

  my_type(const my_type &rhs) : value(rhs.value), tracker(rhs.tracker) {
    tracker.instances++;
    tracker.copies++;
  }

  my_type(my_type &&rhs) : value(rhs.value), tracker(rhs.tracker) {
    tracker.instances++;
    tracker.moves++;
  }

  ~my_type() {
    tracker.instances--;
  }

  my_type & operator =(const my_type &) = delete;

  bool operator <(const my_type &rhs) const {
    return value < rhs.value;
  }

  int value;
  copy_tracker &tracker;
};

suite<> copies("copying", [](auto &_) {
  _.test("copying", []() {
    auto memo = memoize([](const my_type &x) {
      return x.value;
    });

    copy_tracker t;
    memo(my_type(0, t));

    expect(t.instances, equal_to(1u));
    expect(t.copies, equal_to(0u));
    expect(t.moves, equal_to(1u));

    copy_tracker t2;
    memo(my_type(0, t2));

    expect(t2.instances, equal_to(0u));
    expect(t2.copies, equal_to(0u));
    expect(t2.moves, equal_to(0u));
  });
});
