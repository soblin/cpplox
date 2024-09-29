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
  if (left.index() != right.index()) {
    return false;
  }
  // Now both have same Type
  if (helper::is_nil(left)) {
    // right is also Nil
    return true;
  }
  if (helper::is_bool(left)) {
    return as_variant<bool>(left) == as_variant<bool>(right);
  }
  if (helper::is_long(left)) {
    return as_variant<int64_t>(left) == as_variant<int64_t>(right);
  }
  if (helper::is_double(left)) {
    return as_variant<double>(left) == as_variant<double>(right);
  }
  if (helper::is_str(left)) {
    return as_variant<std::string>(left) == as_variant<std::string>(right);
  }
  assert(false);  // LCOV_EXCL_LINE
  return false;
}

}  // namespace expression
}  // namespace lox
