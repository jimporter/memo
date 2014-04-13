#ifndef INC_MEMO_MEMOIZER_HPP
#define INC_MEMO_MEMOIZER_HPP

#include <functional>
#include <map>
#include <type_traits>

namespace memo {

namespace detail {
  // This gets the base function type from pretty much anything it can:
  // C function types, member function types, monomorphic function objects.
  template<typename T, typename Enable = void>
  struct function_signature;

  template<typename T>
  struct function_signature<T, typename std::enable_if<
    std::is_class<T>::value
  >::type> : function_signature<decltype(&T::operator())> {};

  template<typename T, typename Ret, typename ...Args>
  struct function_signature<Ret (T::*)(Args...)> {
    using type = Ret(Args...);
  };

  template<typename T, typename Ret, typename ...Args>
  struct function_signature<Ret (T::*)(Args...) const> {
    using type = Ret(Args...);
  };

  template<typename Ret, typename ...Args>
  struct function_signature<Ret(Args...)> {
    using type = Ret(Args...);
  };

  template<typename Ret, typename ...Args>
  struct function_signature<Ret (&)(Args...)> {
    using type = Ret(Args...);
  };

  template<typename Ret, typename ...Args>
  struct function_signature<Ret (*)(Args...)> {
    using type = Ret(Args...);
  };

  template<typename T>
  struct remove_const_ref : std::remove_const<
    typename std::remove_reference<T>::type
  > {};

  template<typename T, typename U>
  struct is_same_base_type : std::is_same<
    typename remove_const_ref<T>::type,
    typename remove_const_ref<U>::type
  > {};
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
  // This lets us do lookups in our map without converting our value to
  // key_type, which saves us a copy. There's a proposal to make this work for
  // std::unordered_map too.
  using map_type = std::map<tuple_type, return_type, std::less<void>>;

  memoizer() {}
  memoizer(const T &f) : f_(f) {}

  template<typename ...CallArgs>
  return_reference operator ()(CallArgs &&...args)
  {
    // This is a roundabout way of requiring that call is called with arguments
    // of the same basic type as the function (i.e. it will make use of
    // automatic conversions for similar types). This ensures that we always
    // call the *same* function for polymorphic function objects.
    return call<
      typename std::conditional<
        detail::is_same_base_type<CallArgs, Args>::value,
        CallArgs &&,
        typename detail::remove_const_ref<Args>::type &&
      >::type...
    >(std::forward<CallArgs>(args)...);
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

  template<typename ...CallArgs>
  return_reference call(CallArgs &&...args) {
    auto args_tuple = std::forward_as_tuple(std::forward<CallArgs>(args)...);
    auto i = memo_.find(args_tuple);
    if(i != memo_.end()) {
      return i->second;
    }
    else {
      auto result = call_function(args...);
      auto ins = memo_.emplace(
        std::move(args_tuple),
        std::move(result)
      ).first;
      return ins->second;
    }
  }

  template<typename ...CallArgs>
  auto call_function(const CallArgs &...args) -> typename std::enable_if<
    can_pass_memoizer<T, CallArgs...>::value,
    return_type
  >::type {
    return f_(*this, args...);
  }

  template<typename ...CallArgs>
  auto call_function(const CallArgs &...args) -> typename std::enable_if<
    !can_pass_memoizer<T, CallArgs...>::value,
    return_type
  >::type {
    return f_(args...);
  }

  map_type memo_;
  T f_;
};

template<typename Function, typename T>
inline auto memoize(T &&t) {
  return memoizer<T, Function>(std::forward<T>(t));
}

template<typename T>
inline auto memoize(T &&t) {
  return memoizer<T, typename detail::function_signature<T>::type>(
    std::forward<T>(t)
  );
}

} // namespace memo

#endif
