#include <cpplox/debug.hpp>

#include <boost/variant/recursive_variant.hpp>

#include <sstream>

#include <magic_enum.hpp>

namespace lox
{

inline namespace debug
{
class LispReprVisitor : boost::static_visitor<void>
{
public:
  std::stringstream ss;

  void operator()(const Literal & literal) { ss << literal.lexeme; }

  void operator()(const Unary & unary)
  {
    ss << "(" << unary.op.lexeme << " ";
    boost::apply_visitor(*this, unary.expr);
    ss << ")";
  }

  void operator()(const Binary & binary)
  {
    ss << "(" << binary.op.lexeme << " ";
    boost::apply_visitor(*this, binary.left);
    ss << " ";
    boost::apply_visitor(*this, binary.right);
    ss << ")";
  }

  void operator()(const Group & group)
  {
    ss << "(group ";
    boost::apply_visitor(*this, group.expr);
    ss << ")";
  }

  void operator()(const Variable & variable) { ss << variable.name.lexeme; }

  void operator()(const Assign & assign)
  {
    ss << "(= " << assign.name.lexeme << " ";
    boost::apply_visitor(*this, assign.expr);
    ss << ")";
  }
};

auto to_lisp_repr(const Expr & expr) -> std::string
{
  auto visitor = LispReprVisitor();
  boost::apply_visitor(visitor, expr);
  return visitor.ss.str();
}

}  // namespace debug

inline namespace error
{

auto SyntaxError::get_line_string(const size_t offset) const -> std::string
{
  std::stringstream ss;
  if (offset > 0) {
    ss << std::string(offset, ' ');
  }
  ss << "SyntaxError: " << magic_enum::enum_name(kind) << " at line " << line->number << ", column "
     << (ctx_start_index - line->start_index) << std::endl;
  return ss.str();
}

auto SyntaxError::get_visualization_string(
  const std::string_view & source, const size_t offset) const -> std::string
{
  std::stringstream ss;
  if (offset > 0) {
    ss << std::string(offset, ' ');
  }
  ss << debug::Thin << source.substr(line->start_index, ctx_start_index - line->start_index)
     << debug::Reset;
  ss << debug::Bold << debug::Red << debug::Underline
     << source.substr(ctx_start_index, line->end_index - ctx_start_index + 1) << debug::Reset
     << std::endl;
  ss << std::string(offset + ctx_start_index - line->start_index, ' ') << "^" << std::endl;
  return ss.str();
}

}  // namespace error
}  // namespace lox
