#ifndef INC_MEMO_MEMOIZER_HPP
#define INC_MEMO_MEMOIZER_HPP

#include <functional>
#include <map>
#include <type_traits>

namespace memo {

namespace detail {
  template<typename T, typename Enable = void>
  struct dismember;

  template<typename T>
  struct dismember<T, typename std::enable_if<
                        std::is_class<T>::value
                        >::type> : dismember<decltype(&T::operator())> {};

  template<typename T, typename Ret, typename ...Args>
  struct dismember<Ret (T::*)(Args...)> {
    using type = Ret(Args...);
  };

  template<typename T, typename Ret, typename ...Args>
  struct dismember<Ret (T::*)(Args...) const> {
    using type = Ret(Args...);
  };

  template<typename Ret, typename ...Args>
  struct dismember<Ret(Args...)> {
    using type = Ret(Args...);
  };

  template<typename Ret, typename ...Args>
  struct dismember<Ret (&)(Args...)> {
    using type = Ret(Args...);
  };

  template<typename Ret, typename ...Args>
  struct dismember<Ret (*)(Args...)> {
    using type = Ret(Args...);
  };
}

template<typename T, typename Function>
class memoizer;

template<typename T, typename Ret, typename ...Args>
class memoizer<T, Ret(Args...)> {
public:
  using function_type = Ret(Args...);
  using tuple_type = std::tuple<typename std::remove_reference<Args>::type...>;
  using return_type = Ret;
  using return_reference = const return_type &;
  using map_type = std::map<tuple_type, return_type>;

  memoizer() {}
  memoizer(const T &f) : f_(f) {}

  return_reference operator ()(const Args &...args) {
    auto args_tuple = std::forward_as_tuple(args...);
    auto i = memo_.find(args_tuple);
    if(i != memo_.end()) {
      return i->second;
    }
    else {
      auto ins = memo_.emplace(args_tuple, call(args...)).first;
      return ins->second;
    }
  }
private:
  template<typename T2, typename ...Args2>
  class can_pass_memoizer {
    template<typename U> struct always_bool { typedef bool type; };

    template<typename T3, typename ...Args3>
    static constexpr typename always_bool<
      decltype(std::declval<T3>()(
        std::declval<memoizer&>(), std::declval<Args3>()...
      ))
    >::type check_(int) {
      return true;
    }
    template<typename T3, typename ...Args3>
    static constexpr bool check_(...) {
      return false;
    }
  public:
    static const bool value = check_<T2, Args2...>(0);
  };

  template<typename ...InnerArgs>
  auto call(const InnerArgs &...args) -> typename std::enable_if<
    can_pass_memoizer<T, InnerArgs...>::value,
    return_type
  >::type {
    return f_(*this, args...);
  }

  template<typename ...InnerArgs>
  auto call(const InnerArgs &...args) -> typename std::enable_if<
    !can_pass_memoizer<T, InnerArgs...>::value,
    return_type
  >::type {
    return f_(args...);
  }

  map_type memo_;
  T f_;
};

template<typename Function, typename T>
auto memoize(T &&t) {
  return memoizer<T, Function>(std::forward<T>(t));
}

template<typename T>
auto memoize(T &&t) {
  return memoizer<T, typename detail::dismember<T>::type>(std::forward<T>(t));
}

} // namespace memo

#endif
