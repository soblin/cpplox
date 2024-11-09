#include <cpplox/expression.hpp>
#include <cpplox/variant.hpp>

namespace lox
{

inline namespace expression
{

auto is_truthy(const Value & value) -> bool
{
  if (is_variant_v<Nil>(value)) {
    return false;
  }
  if (is_variant_v<bool>(value)) {
    return as_variant<bool>(value);
  }
  return true;
}

auto is_equal(const Value & left, const Value & right) -> bool
{
  // Now both have same Type
  if (helper::is_nil(left) && helper::is_nil(right)) {
    // right is also Nil
    return true;
  }
  if (helper::is_bool(left) && helper::is_bool(right)) {
    return as_variant<bool>(left) == as_variant<bool>(right);
  }
  if (helper::is_long(left) && helper::is_long(right)) {
    return as_variant<int64_t>(left) == as_variant<int64_t>(right);
  }
  if (helper::is_double(left) && helper::is_double(right)) {
    return as_variant<double>(left) == as_variant<double>(right);
  }
  if (helper::is_str(left) && helper::is_str(right)) {
    return as_variant<std::string>(left) == as_variant<std::string>(right);
  }
  return false;
}

}  // namespace expression
}  // namespace lox
