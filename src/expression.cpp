#include <cpplox/expression.hpp>
#include <cpplox/variant.hpp>

namespace lox
{

inline namespace expression
{

class LispReprVisitor : boost::static_visitor<std::string>
{
public:
  std::string operator()(const Literal & literal) { return literal.lexeme; }
  std::string operator()(const Unary & unary)
  {
    return "(" + unary.op.lexeme + " " + boost::apply_visitor(LispReprVisitor(), unary.expr) + ")";
  }
  std::string operator()(const Binary & binary)
  {
    return "(" + binary.op.lexeme + " " + boost::apply_visitor(LispReprVisitor(), binary.left) +
           " " + boost::apply_visitor(LispReprVisitor(), binary.right) + ")";
  }
  std::string operator()(const Group & group)
  {
    return "(group " + boost::apply_visitor(LispReprVisitor(), group.expr) + ")";
  }
  std::string operator()(const Variable & variable) { return variable.name.lexeme; }
};

auto to_lisp_repr(const Expr & expr) -> std::string
{
  return boost::apply_visitor(LispReprVisitor(), expr);
}

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
  assert(false);
  return false;
}

}  // namespace expression
}  // namespace lox
