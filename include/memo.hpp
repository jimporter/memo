#ifndef INC_MEMO_HPP
#define INC_MEMO_HPP

#include <functional>
#include <map>
#include <type_traits>

namespace memo {

  namespace detail {

    // This is identical to the C++20 type trait.
    template<typename T>
    struct remove_cvref : std::remove_cv<std::remove_reference_t<T>> {};

    template<typename T>
    using remove_cvref_t = typename remove_cvref<T>::type;

    template<typename T>
    struct drop_first_arg;

    template<typename Ret, typename First, typename ...Args>
    struct drop_first_arg<Ret(First, Args...)> {
      using type = Ret(Args...);
    };

    struct anything {
      template<typename T> operator T();
      template<typename T> anything operator +(T &&);
      template<typename T> anything operator -(T &&);
      template<typename T> anything operator *(T &&);
      template<typename T> anything operator /(T &&);
    };

    // This gets the base function type from pretty much anything it can:
    // C function types, member function types, monomorphic function objects.
    template<typename T, typename Enable = void>
    struct function_signature;

    template<typename T>
    struct function_signature<T, std::enable_if_t<
      std::is_class_v<remove_cvref_t<T>>
    >> : function_signature<decltype(&remove_cvref_t<T>::operator())> {};

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

    template<typename T, typename Result = anything, typename Enable = void>
    struct recursive_function_signature;

    template<typename T, typename Result>
    struct recursive_function_signature<
      T, Result, std::enable_if_t<std::is_class_v<remove_cvref_t<T>>>
    > {
      using type = typename drop_first_arg<function_signature_t<decltype(
        &remove_cvref_t<T>::template operator()<Result(*)(...)>
      )>>::type;
    };

    template<typename ...T>
    using recursive_function_signature_t =
      typename recursive_function_signature<T...>::type;

    template<typename T, typename U>
    struct is_same_underlying_type : std::is_same<
      remove_cvref_t<T>, remove_cvref_t<U>
    > {};

    template<typename T, typename U>
    inline constexpr bool is_same_underlying_type_v =
      is_same_underlying_type<T, U>::value;

  }

  template<typename Signature, typename Function, bool Recursive = false>
  class memoizer;

  template<typename Ret, typename ...Args, typename Function, bool Recursive>
  class memoizer<Ret(Args...), Function, Recursive> {
  public:
    using function_type = Function;
    using function_signature = Ret(Args...);
    using tuple_type = std::tuple<std::remove_reference_t<Args>...>;
    using return_type = Ret;
    using return_reference = const return_type &;
    // This lets us do lookups in our map without converting our value to
    // key_type, which saves us a copy. There's a proposal to make this work for
    // std::unordered_map too.
    using map_type = std::map<tuple_type, return_type, std::less<void>>;

    memoizer() {}
    memoizer(const function_type &f) : f_(f) {}

    template<typename ...CallArgs>
    return_reference operator ()(CallArgs &&...args) {
      // This is a roundabout way of requiring that call is called with
      // arguments of the same basic type as the function (i.e. it will make use
      // of automatic conversions for similar types). This ensures that we
      // always call the *same* function for polymorphic function objects.
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
    template<typename ...CallArgs>
    return_reference call(CallArgs ...args) {
      auto args_tuple = std::forward_as_tuple(std::forward<CallArgs>(args)...);
      auto i = memo_.find(args_tuple);
      if(i != memo_.end()) {
        return i->second;
      } else {
        if constexpr(Recursive) {
          return store_call(
            [this](auto &&...args) {
              return f_(*this, std::forward<decltype(args)>(args)...);
            },
            std::move(args_tuple)
          )->second;
        } else {
          return store_call(f_, std::move(args_tuple))->second;
        }
      }
    }

    template<typename F, typename ArgsTuple>
    auto store_call(F &&f, ArgsTuple &&args) {
      auto result = std::apply(std::forward<F>(f), args);
      return memo_.emplace(
        std::forward<ArgsTuple>(args),
        std::move(result)
      ).first;
    }

    map_type memo_;
    function_type f_;
  };

  template<typename Signature, typename T>
  inline auto memoize(T &&t) {
    return memoizer<Signature, T>(std::forward<T>(t));
  }

  template<typename T>
  inline auto memoize(T &&t) {
    return memoizer<detail::function_signature_t<T>, T>(std::forward<T>(t));
  }

  template<typename Signature, typename T>
  inline auto recursive_memoize(T &&t) {
    if constexpr(std::is_function_v<Signature>) {
      return memoizer<Signature, T, true>(std::forward<T>(t));
    } else {
      using Sig = detail::recursive_function_signature_t<T, Signature>;
      return memoizer<Sig, T, true>(std::forward<T>(t));
    }
  }

  template<typename T>
  inline auto recursive_memoize(T &&t) {
    return memoizer<detail::recursive_function_signature_t<T>, T, true>(
      std::forward<T>(t)
    );
  }

} // namespace memo

#endif
