#include <cpplox/debug.hpp>

#include <boost/variant/recursive_variant.hpp>

namespace lox
{

inline namespace debug
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

}  // namespace debug
}  // namespace lox
