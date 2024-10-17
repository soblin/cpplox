#include <cpplox/debug.hpp>
#include <cpplox/statement.hpp>

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

  void operator()(const Logical & logical)
  {
    if (logical.op.type == TokenType::And) {
      ss << "(and ";
    } else if (logical.op.type == TokenType::Or) {
      ss << "(or ";
    }
    boost::apply_visitor(*this, logical.left);
    boost::apply_visitor(*this, logical.right);
    ss << ")";
  }

  void operator()(const Call & call)
  {
    boost::apply_visitor(*this, call.callee);
    ss << "(";
    for (const auto & argument : call.arguments) {
      boost::apply_visitor(*this, argument);
      ss << ", ";
    }
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
     << source.substr(ctx_start_index, line->end_index - ctx_start_index + 1) << debug::Reset;
  if (source.at(line->end_index) != '\n') {
    ss << std::endl;
  }
  ss << std::string(offset + ctx_start_index - line->start_index, ' ') << "^" << std::endl;
  return ss.str();
}

class ExprRangeVisitor : boost::static_visitor<std::pair<Token, Token>>
{
public:
  std::pair<Token, Token> operator()(const Literal & literal)
  {
    return {static_cast<Token>(literal), static_cast<Token>(literal)};
  }

  std::pair<Token, Token> operator()(const Unary & unary)
  {
    const auto [ignore, right_end] = boost::apply_visitor(*this, unary.expr);
    return {unary.op, right_end};
  }

  std::pair<Token, Token> operator()(const Binary & binary)
  {
    const auto [left_start, ignore1] = boost::apply_visitor(*this, binary.left);
    const auto [ignore2, right_end] = boost::apply_visitor(*this, binary.right);
    return {left_start, right_end};
  }

  std::pair<Token, Token> operator()(const Group & group)
  {
    return {group.left_paren, group.right_paren};
  }

  std::pair<Token, Token> operator()(const Variable & var) { return {var.name, var.name}; }

  std::pair<Token, Token> operator()(const Assign & assign)
  {
    const auto [ignore, right_end] = boost::apply_visitor(*this, assign.expr);
    return {assign.name, right_end};
  }

  std::pair<Token, Token> operator()(const Logical & logical)
  {
    const auto [left_start, ignore1] = boost::apply_visitor(*this, logical.left);
    const auto [ignore2, right_end] = boost::apply_visitor(*this, logical.right);
    return {left_start, right_end};
  }

  std::pair<Token, Token> operator()(const Call & call)
  {
    const auto [left_start, left_end] = boost::apply_visitor(*this, call.callee);
    if (call.arguments.empty()) {
      return {left_start, left_end};
    }
    const auto [right_start, right_end] = boost::apply_visitor(*this, call.arguments.back());
    return {left_start, right_end};
  }
};

auto get_line_string(const RuntimeError & error, const size_t offset) -> std::string
{
  std::stringstream ss;
  if (offset > 0) {
    ss << std::string(offset, ' ');
  }
  ss << "RuntimeError: ";
  if (is_variant_v<TypeError>(error)) {
    const auto & err = as_variant<TypeError>(error);
    ss << "TypeError at line " << err.op.line->number << ", column "
       << (err.op.start_index + err.op.lexeme.size() - err.op.line->start_index + 1) << std::endl;
    return ss.str();
  }
  if (is_variant_v<UndefinedVariableError>(error)) {
    const auto & err = as_variant<UndefinedVariableError>(error);
    ss << "undefined variable '" << err.variable.lexeme << "' at line " << err.variable.line->number
       << ", column "
       << (err.variable.start_index + err.variable.lexeme.size() - err.variable.line->number + 1)
       << std::endl;
    return ss.str();
  }
  if (is_variant_v<MaxLoopError>(error)) {
    const auto & err = as_variant<MaxLoopError>(error);
    ss << "exceeded maximum loop limit from '" << err.token.lexeme << "' statement at "
       << err.token.line->number << ", column "
       << (err.token.start_index + err.token.lexeme.size() - err.token.line->number + 1)
       << std::endl;
    return ss.str();
  }
  if (is_variant_v<NotInvocableError>(error)) {
    const auto & err = as_variant<NotInvocableError>(error);
    const auto [start, end] = boost::apply_visitor(ExprRangeVisitor(), err.callee);
    ss << "could not call " << start.lexeme << " statement at " << start.line->number << ", column "
       << (start.start_index + start.lexeme.size() - start.line->number + 1) << " because "
       << err.desc << std::endl;
    return ss.str();
  }
  if (is_variant_v<NoReturnFromFunction>(error)) {
    const auto & err = as_variant<NoReturnFromFunction>(error);
    const auto func_name_decl = err.callee.definition->name;
    ss << "function " << func_name_decl.lexeme << " define at line " << func_name_decl.line->number
       << ", column "
       << (func_name_decl.start_index + func_name_decl.lexeme.size() -
           func_name_decl.line->start_index + 1)
       << " does not return" << std::endl;
    return ss.str();
  }
  assert(false);
}

static auto get_visualization_string_expr(
  const std::string & source, const Token & op, const Expr & expr,
  const size_t offset) -> std::string
{
  std::stringstream ss;
  if (offset > 0) {
    ss << std::string(offset, ' ');
  }
  const auto [expr_start, expr_end] = boost::apply_visitor(ExprRangeVisitor(), expr);
  // source(expr_start.line->start_index ~ expr_start.start_index): Thin
  // source(expr_start.start_index+1 ~ op.start_index-1): underline
  // source(op.start_index ~ op.start_index + op.lexeme.size()): red bold underline
  // source(op.start_index + lexeme.size() + 1, expr_end.start_index + expr_end.lexeme.size()):
  // underline source(expr_end.start_index + expr_end.lexeme.size() + 1,
  // expr_end.line->end_index): Thin
  ss << debug::Thin
     << source.substr(
          expr_start.line->start_index, expr_start.start_index - expr_start.line->start_index)
     << debug::Reset;
  ss << debug::Underline
     << source.substr(expr_start.start_index, op.start_index - (expr_start.start_index))
     << debug::Reset;
  ss << debug::Underline << debug::Red << debug::Bold
     << source.substr(op.start_index, op.lexeme.size()) << debug::Reset;
  ss << debug::Underline
     << source.substr(
          op.start_index + op.lexeme.size(),
          expr_end.start_index + expr_end.lexeme.size() - (op.start_index + op.lexeme.size()) + 1)
     << debug::Reset;
  ss << debug::Thin
     << source.substr(
          expr_end.start_index + expr_end.lexeme.size() + 1,
          expr_end.line->end_index - (expr_end.start_index + expr_end.lexeme.size()))
     << debug::Reset;
  if (source.at(expr_end.line->end_index) != '\n') {
    ss << std::endl;
  }
  ss << std::string(offset + (op.start_index - op.line->start_index), ' ') << "^" << std::endl;
  return ss.str();
}

auto get_visualization_string(
  const std::string_view & source, const Token & from, const Token & to,
  const size_t offset) -> std::string
{
  std::stringstream ss;
  if (offset > 0) {
    ss << std::string(offset, ' ');
  }
  ss << debug::Thin
     << source.substr(from.line->start_index, from.start_index - from.line->start_index)
     << debug::Reset;
  ss << debug::Underline << debug::Red << debug::Bold
     << source.substr(from.start_index, to.start_index - from.start_index + to.lexeme.size())
     << debug::Reset;
  ss << debug::Thin
     << source.substr(
          to.start_index + to.lexeme.size(),
          to.line->end_index - (to.start_index + to.lexeme.size()))
     << debug::Reset;
  ss << std::endl;
  ss << std::string(offset + (from.start_index - from.line->start_index), ' ')
     << std::string(to.start_index - from.start_index + to.lexeme.size(), '^') << std::endl;
  return ss.str();
}

auto get_visualization_string(
  const std::string & source, const RuntimeError & error, const size_t offset) -> std::string
{
  std::stringstream ss;
  if (offset > 0) {
    ss << std::string(offset, ' ');
  }
  if (is_variant_v<TypeError>(error)) {
    const auto & err = as_variant<TypeError>(error);
    const auto [expr_start, expr_end] = boost::apply_visitor(ExprRangeVisitor(), err.expr);
    return get_visualization_string_expr(source, err.op, err.expr, offset);
  }
  if (is_variant_v<UndefinedVariableError>(error)) {
    const auto & err = as_variant<UndefinedVariableError>(error);
    const auto [expr_start, expr_end] = boost::apply_visitor(ExprRangeVisitor(), err.expr);
    return get_visualization_string_expr(source, err.variable, err.expr, offset);
  }
  if (is_variant_v<MaxLoopError>(error)) {
    const auto & err = as_variant<MaxLoopError>(error);
    if (err.cond) {
      const auto [expr_start, expr_end] =
        boost::apply_visitor(ExprRangeVisitor(), err.cond.value());
      return get_visualization_string(source, err.token, expr_end, offset);
    }
    return get_visualization_string(source, err.token, err.token, offset);
  }
  if (is_variant_v<NotInvocableError>(error)) {
    const auto & err = as_variant<NotInvocableError>(error);
    const auto [start, end] = boost::apply_visitor(ExprRangeVisitor(), err.callee);
    return get_visualization_string(source, start, end, offset);
  }
  if (is_variant_v<NoReturnFromFunction>(error)) {
    const auto & err = as_variant<NoReturnFromFunction>(error);
    return get_visualization_string(
      source, err.callee.definition->name, err.callee.definition->name, offset);
  }
  assert(false);
}

}  // namespace error
}  // namespace lox
