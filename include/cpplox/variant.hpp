#pragma once

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

template <typename T, typename V>
auto move_as_variant(V && v) noexcept ->
  typename std::enable_if_t<detail::is_within_variant_v<T, V>, T>
{
  T & value = std::get<T>(std::forward<V>(v));
  return T(std::move(value));
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
