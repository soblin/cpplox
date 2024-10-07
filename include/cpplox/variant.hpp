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

template <class T>
struct remove_cvref
{
  using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

template <typename T, typename V>
struct is_within_variant;

template <typename T, typename Head, typename... Tail>
struct is_within_variant<T, std::variant<Head, Tail...>>
: std::conditional_t<
    std::is_same_v<T, Head>, std::true_type, is_within_variant<T, std::variant<Tail...>>>
{
};

template <typename T>
struct is_within_variant<T, std::variant<>> : std::false_type
{
};

template <typename T, typename Head, typename... Tail>
struct is_within_variant<T, boost::variant<Head, Tail...>>
: std::conditional_t<
    std::is_same_v<T, Head>, std::true_type, is_within_variant<T, boost::variant<Tail...>>>
{
};

template <typename T, typename Head>
struct is_within_variant<T, boost::variant<Head>>
: std::conditional_t<std::is_same_v<T, Head>, std::true_type, std::false_type>
{
};

template <typename T, typename V>
static constexpr bool is_within_variant_v =
  is_within_variant<typename std::decay_t<T>, typename std::decay_t<V>>::value;
}  // namespace detail

/**
 * @brief statically check if the specified type is the candidate of the given variant and return
 * true if the internal hold type matches given type
 */
template <typename T, typename V>
auto is_variant_v(V && v) -> typename std::enable_if_t<detail::is_within_variant_v<T, V>, bool>
{
  return std::holds_alternative<T>(std::forward<V>(v));
}

namespace experimental
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

template <typename T, typename V>
static constexpr bool is_within = is_variant<V>::template is_within<T>;

template <typename T, typename V>
auto is_variant_v(V && v) -> typename std::enable_if_t<is_within<T, V>, bool>
{
  if constexpr (checker_for_std<V>::value) {
    return std::holds_alternative<T>(v);
  }
  if constexpr (checker_for_boost<V>::value) {
    return boost::get<T>(&v) != nullptr;
  }
}

template <typename T, typename V>
auto as_variant(V && v) -> typename std::enable_if_t<is_within<T, V>, const T &>
{
  if constexpr (checker_for_std<V>::value) {
    return std::get<T>(std::forward<V>(v));
  }
  if constexpr (checker_for_boost<V>::value) {
    return boost::get<T>(v);
  }
}
}  // namespace experimental

/**
 * @brief statically check if the specified type is the candidate of the given variant and return
 * the reference to the internal hold data as specified type
 */
template <typename T, typename V>
auto as_variant(V && v) noexcept ->
  typename std::enable_if_t<detail::is_within_variant_v<T, V>, const T &>
{
  return std::get<T>(std::forward<V>(v));
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
