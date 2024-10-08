#pragma once

#include <boost/variant.hpp>

#include <type_traits>
#include <utility>
#include <variant>

namespace lox
{
inline namespace variant
{

namespace detail
{
template <typename T>
struct checker_for_std : std::false_type
{
};

template <typename... Ts>
struct checker_for_std<std::variant<Ts...>> : std::true_type
{
};

template <typename V, typename Enable = void>
struct is_variant;

template <template <typename...> class Variant, typename... Ts>
struct is_variant<
  Variant<Ts...>, typename std::enable_if<checker_for_std<Variant<Ts...>>::value>::type>
{
  template <typename X, typename Y>
  struct check;

  template <typename T, typename Head, typename... Tail>
  struct check<T, Variant<Head, Tail...>>
  : std::conditional_t<std::is_same_v<T, Head>, std::true_type, check<T, Variant<Tail...>>>
  {
  };

  template <typename T, typename Head>
  struct check<T, Variant<Head>>
  : std::conditional_t<std::is_same_v<T, Head>, std::true_type, std::false_type>
  {
  };

  template <typename T>
  static constexpr bool is_within = check<T, Variant<Ts...>>::value;
};

template <typename T>
struct checker_for_boost : std::false_type
{
};

template <typename... Ts>
struct checker_for_boost<boost::variant<Ts...>> : std::true_type
{
};

template <template <typename...> class Variant, typename... Ts>
struct is_variant<
  Variant<Ts...>, typename std::enable_if<checker_for_boost<Variant<Ts...>>::value>::type>
{
  template <typename X, typename Y>
  struct check;

  template <typename T, typename Head, typename... Tail>
  struct check<T, Variant<Head, Tail...>>
  : std::conditional_t<
      std::disjunction<
        std::is_same<T, Head>,                           //<! normal case OR
        std::is_same<boost::recursive_wrapper<T>, Head>  //<! recursive variant
        >::value,
      std::true_type, check<T, Variant<Tail...>>>
  {
  };

  template <typename T, typename Head>
  struct check<T, Variant<Head>>
  : std::conditional_t<
      std::disjunction<
        std::is_same<T, Head>,                           //<! normal case OR
        std::is_same<boost::recursive_wrapper<T>, Head>  //<! recursive variant
        >::value,
      std::true_type, std::false_type>
  {
  };

  template <typename T>
  static constexpr bool is_within = check<T, Variant<Ts...>>::value;
};

template <typename T, typename V>
static constexpr bool is_within = is_variant<V>::template is_within<T>;

}  // namespace detail

/**
 * @brief statically check if the specified type is the candidate of the given variant and return
 * true if the internal hold type matches given type
 */
template <typename T, typename V>
auto is_variant_v(const V & v) ->
  typename std::enable_if_t<
    detail::is_within<typename std::decay_t<T>, typename std::decay_t<V>>, bool>
{
  if constexpr (detail::checker_for_std<typename std::decay_t<V>>::value) {
    return std::holds_alternative<T>(v);
  }
  if constexpr (detail::checker_for_boost<typename std::decay_t<V>>::value) {
    return boost::get<T>(&v) != nullptr;
  }
}

/**
 * @brief statically check if the specified type is the candidate of the given variant and return
 * the reference to the internal hold data as specified type
 */
template <typename T, typename V>
auto as_variant(V && v) ->
  typename std::enable_if_t<
    detail::is_within<typename std::decay_t<T>, typename std::decay_t<V>>, const T &>
{
  if constexpr (detail::checker_for_std<typename std::decay_t<V>>::value) {
    return std::get<T>(std::forward<V>(v));
  }
  if constexpr (detail::checker_for_boost<typename std::decay_t<V>>::value) {
    return boost::get<T>(v);
  }
}

template <class... Ts>
struct visit_variant : Ts...
{
  using Ts::operator()...;
};

template <class... Ts>
visit_variant(Ts...) -> visit_variant<Ts...>;

}  // namespace variant
}  // namespace lox
