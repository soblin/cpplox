#include <cpplox/expression.hpp>
#include <cpplox/variant.hpp>

#include <boost/lexical_cast.hpp>

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
};

auto to_lisp_repr(const Expr & expr) -> std::string
{
  return boost::apply_visitor(LispReprVisitor(), expr);
}

class EvaluateExprVisitor : boost::static_visitor<std::variant<Value, InterpretError>>
{
public:
  std::variant<Value, InterpretError> operator()(const Literal & literal)
  {
    if (literal.type == TokenType::Nil) {
      return expression::Nil{};
    }
    if (literal.type == TokenType::False) {
      return false;
    }
    if (literal.type == TokenType::True) {
      return true;
    }
    if (literal.type == TokenType::String) {
      return literal.lexeme;
    }
    if (literal.type == TokenType::Number) {
      return boost::lexical_cast<double>(literal.lexeme);
    }

    // this is unreachable actually
    assert(false);
    return expression::Nil{};
  }

  std::variant<Value, InterpretError> operator()(const Unary & unary)
  {
    const auto right = boost::apply_visitor(EvaluateExprVisitor(), unary.expr);
    if (is_variant_v<InterpretError>(right)) {
      return right;
    }
    const auto & right_value = as_variant<Value>(right);
    if (unary.op.type == TokenType::Minus) {
      if (!is_variant_v<double>(right_value)) {
        // TODO(soblin): like -"123"
        return InterpretError{InterpretErrorKind::TypeError};
      }
      return -1.0 * as_variant<double>(right_value);
    }
    if (unary.op.type == TokenType::Bang) {
      return !is_truthy(right_value);
    }

    // this is unreachable actually
    assert(false);
    return expression::Nil{};
  }

  std::variant<Value, InterpretError> operator()(const Binary & binary)
  {
    const auto left_opt = boost::apply_visitor(EvaluateExprVisitor(), binary.left);
    if (is_variant_v<InterpretError>(left_opt)) {
      return left_opt;
    }
    const auto right_opt = boost::apply_visitor(EvaluateExprVisitor(), binary.right);
    if (is_variant_v<InterpretError>(right_opt)) {
      return right_opt;
    }
    const auto & left = as_variant<Value>(left_opt);
    const auto & right = as_variant<Value>(right_opt);

    // A * B
    if (binary.op.type == TokenType::Star) {
      if (left.index() != helper::double_Index || right.index() != helper::double_Index) {
        // TODO(soblin): like "123" * true
        return InterpretError{InterpretErrorKind::TypeError};
      }
      return as_variant<double>(left) * as_variant<double>(right);
    }

    // A / B
    if (binary.op.type == TokenType::Slash) {
      if (left.index() != helper::double_Index || right.index() != helper::double_Index) {
        // TODO(soblin): like "123" * true
        return InterpretError{InterpretErrorKind::TypeError};
      }
      // TODO(soblin): ZeroDivisionError
      return as_variant<double>(left) / as_variant<double>(right);
    }

    // A - B
    if (binary.op.type == TokenType::Minus) {
      if (left.index() != helper::double_Index || right.index() != helper::double_Index) {
        // TODO(soblin): like "123" * true
        return InterpretError{InterpretErrorKind::TypeError};
      }
      return as_variant<double>(left) - as_variant<double>(right);
    }

    // A + B
    if (binary.op.type == TokenType::Plus) {
      if (left.index() == helper::double_Index && right.index() == helper::double_Index) {
        return as_variant<double>(left) + as_variant<double>(right);
      }
      if (left.index() == helper::str_Index && right.index() == helper::str_Index) {
        return as_variant<std::string>(left) + as_variant<std::string>(right);
      }
      return InterpretError{InterpretErrorKind::TypeError};
    }

    // A > B
    if (binary.op.type == TokenType::Greater) {
      if (left.index() == helper::double_Index && right.index() == helper::double_Index) {
        return as_variant<double>(left) > as_variant<double>(right);
      }
      return InterpretError{InterpretErrorKind::TypeError};
    }

    // A >= B
    if (binary.op.type == TokenType::GreaterEqual) {
      if (left.index() == helper::double_Index && right.index() == helper::double_Index) {
        return as_variant<double>(left) >= as_variant<double>(right);
      }
      return InterpretError{InterpretErrorKind::TypeError};
    }

    // A < B
    if (binary.op.type == TokenType::Less) {
      if (left.index() == helper::double_Index && right.index() == helper::double_Index) {
        return as_variant<double>(left) < as_variant<double>(right);
      }
      return InterpretError{InterpretErrorKind::TypeError};
    }

    // A <= B
    if (binary.op.type == TokenType::LessEqual) {
      if (left.index() == helper::double_Index && right.index() == helper::double_Index) {
        return as_variant<double>(left) <= as_variant<double>(right);
      }
      return InterpretError{InterpretErrorKind::TypeError};
    }

    // A == B
    if (binary.op.type == TokenType::EqualEqual) {
      return is_equal(left, right);
    }

    // A != B
    if (binary.op.type == TokenType::EqualEqual) {
      return !is_equal(left, right);
    }

    // this is unreachable actually
    assert(false);
    return expression::Nil{};
  }

  std::variant<Value, InterpretError> operator()(const Group & group)
  {
    return boost::apply_visitor(EvaluateExprVisitor(), group.expr);
  }
};

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
  if (is_variant_v<Nil>(left)) {
    // right is also Nil
    return true;
  }
  if (is_variant_v<bool>(left)) {
    return as_variant<bool>(left) == as_variant<bool>(right);
  }
  if (is_variant_v<double>(left)) {
    return as_variant<double>(left) == as_variant<double>(right);
  }
  if (is_variant_v<std::string>(left)) {
    return as_variant<std::string>(left) == as_variant<std::string>(right);
  }
  assert(false);
  return false;
}

auto evaluate_expr(const Expr & expr) -> std::variant<Value, InterpretError>
{
  return boost::apply_visitor(EvaluateExprVisitor(), expr);
}

}  // namespace expression
}  // namespace lox
