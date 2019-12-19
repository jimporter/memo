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
  struct function_signature<T, std::enable_if_t<std::is_class_v<T>>>
    : function_signature<decltype(&T::operator())> {};

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
  using function_signature_t = typename function_signature<T>::type;

  // This is identical to the C++20 type trait.
  template<typename T>
  struct remove_cvref : std::remove_cv<std::remove_reference_t<T>> {};

  template<typename T>
  using remove_cvref_t = typename remove_cvref<T>::type;

  template<typename T, typename U>
  struct is_same_underlying_type : std::is_same<
    remove_cvref_t<T>, remove_cvref_t<U>
  > {};

  template<typename T, typename U>
  inline constexpr bool is_same_underlying_type_v =
    is_same_underlying_type<T, U>::value;
}

template<typename T, typename Function>
class memoizer;

template<typename T, typename Ret, typename ...Args>
class memoizer<T, Ret(Args...)> {
public:
  using function_type = T;
  using function_signature = Ret(Args...);
  using tuple_type = std::tuple<std::remove_reference_t<Args>...>;
  using return_type = Ret;
  using return_reference = const return_type &;
  // This lets us do lookups in our map without converting our value to
  // key_type, which saves us a copy. There's a proposal to make this work for
  // std::unordered_map too.
  using map_type = std::map<tuple_type, return_type, std::less<void>>;

  memoizer() {}
  memoizer(const T &f) : f_(f) {}

  template<typename ...CallArgs>
  return_reference operator ()(CallArgs &&...args) {
    // This is a roundabout way of requiring that call is called with arguments
    // of the same basic type as the function (i.e. it will make use of
    // automatic conversions for similar types). This ensures that we always
    // call the *same* function for polymorphic function objects.
    return call<std::conditional_t<
      detail::is_same_underlying_type_v<CallArgs, Args>,
      CallArgs &&,
#if defined(_MSC_VER) && !defined(__clang__)
      detail::remove_cvref_t<Args>
#else
      detail::remove_cvref_t<Args> &&
#endif
    >...>(std::forward<CallArgs>(args)...);
  }
private:
  template<typename ...>
  static auto check_can_pass_(...) -> std::false_type;

  template<typename ...CallArgs>
  static auto check_can_pass_(int) -> decltype(
    std::declval<T>()(std::declval<memoizer&>(), std::declval<CallArgs>()...),
    std::true_type()
  );

  template<typename ...CallArgs>
  struct can_pass_memoizer : decltype(check_can_pass_<CallArgs...>(0)) {};

  template<typename ...CallArgs>
  return_reference call(CallArgs ...args) {
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
  auto call_function(const CallArgs &...args) -> std::enable_if_t<
    can_pass_memoizer<CallArgs...>::value, return_type
  > {
    return f_(*this, args...);
  }

  template<typename ...CallArgs>
  auto call_function(const CallArgs &...args) -> std::enable_if_t<
    !can_pass_memoizer<CallArgs...>::value, return_type
  > {
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
  return memoizer<T, detail::function_signature_t<T>>(
    std::forward<T>(t)
  );
}

} // namespace memo

#endif
