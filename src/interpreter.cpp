#include <cpplox/debug.hpp>
#include <cpplox/environment.hpp>
#include <cpplox/expression.hpp>
#include <cpplox/interpreter.hpp>
#include <cpplox/statement.hpp>

#include <boost/lexical_cast.hpp>

#include <functional>
#include <iostream>

namespace lox
{

inline namespace interpreter
{

auto Interpreter::execute(const Program & program) -> std::optional<RuntimeError>
{
  // LCOV_EXCL_START
  auto stringify = [](const Value & value) -> std::string {
    return std::visit(
      visit_variant{
        [](const Nil & nil) -> std::string { return "nil"; },
        [](const bool & boolean) -> std::string { return boolean ? "true" : "false"; },
        [](const int64_t & i) -> std::string { return std::to_string(i); },
        [](const double & d) -> std::string { return std::to_string(d); },
        [](const std::string & str) -> std::string { return str; },
      },
      value);
  };
  // LCOV_EXCL_STOP
  for (const auto & statement : program) {
    const std::optional<RuntimeError> result = std::visit(
      visit_variant{
        [&](const ExprStmt & stmt) -> std::optional<RuntimeError> {
          const auto eval_opt = evaluate_expr(stmt.expression);
          if (is_variant_v<RuntimeError>(eval_opt)) {
            std::cerr << lox::to_lisp_repr(stmt.expression) << std::endl;
            return as_variant<RuntimeError>(eval_opt);
          }
          return std::nullopt;
        },
        [&](const PrintStmt & stmt) -> std::optional<RuntimeError> {
          const auto eval_opt = evaluate_expr(stmt.expression);
          if (is_variant_v<RuntimeError>(eval_opt)) {
            return as_variant<RuntimeError>(eval_opt);
          }
          // LCOV_EXCL_START
          std::cout << stringify(as_variant<Value>(eval_opt)) << std::endl;
          // LCOV_EXCL_STOP
          return std::nullopt;
        },
        [&](const VarDeclStmt & stmt) -> std::optional<RuntimeError> {
          if (stmt.initializer) {
            const auto eval_opt = evaluate_expr(stmt.initializer.value());
            if (is_variant_v<RuntimeError>(eval_opt)) {
              return as_variant<RuntimeError>(eval_opt);
            }
            env_->define(stmt.name, as_variant<Value>(eval_opt));
          } else {
            env_->define(stmt.name, Nil{});
          }
          return std::nullopt;
        },
      },
      statement);
    if (result) {
      return result.value();
    }
  }
  return std::nullopt;
}

template <template <typename> class F>
auto apply_binary_op_scalar(const Value & left_numeric, const Value & right_numeric) -> Value
{
  if (helper::is_long(left_numeric)) {
    return helper::is_long(right_numeric)
             ? Value{static_cast<int64_t>(F<int64_t>()(
                 as_variant<int64_t>(left_numeric), as_variant<int64_t>(right_numeric)))}
             : Value{static_cast<double>(F<double>()(
                 as_variant<int64_t>(left_numeric), as_variant<double>(right_numeric)))};
  }
  return helper::is_long(right_numeric)
           ? Value{static_cast<double>(
               F<double>()(as_variant<double>(left_numeric), as_variant<int64_t>(right_numeric)))}
           : Value{static_cast<double>(
               F<double>()(as_variant<double>(left_numeric), as_variant<double>(right_numeric)))};
}

template <template <typename> class F>
auto apply_binary_op_bool(const Value & left_numeric, const Value & right_numeric) -> Value
{
  if (helper::is_long(left_numeric)) {
    return helper::is_long(right_numeric)
             ? Value{static_cast<bool>(F<int64_t>()(
                 as_variant<int64_t>(left_numeric), as_variant<int64_t>(right_numeric)))}
             : Value{static_cast<bool>(F<double>()(
                 as_variant<int64_t>(left_numeric), as_variant<double>(right_numeric)))};
  }
  return helper::is_long(right_numeric)
           ? Value{static_cast<bool>(
               F<double>()(as_variant<double>(left_numeric), as_variant<int64_t>(right_numeric)))}
           : Value{static_cast<bool>(
               F<double>()(as_variant<double>(left_numeric), as_variant<double>(right_numeric)))};
}

class EvaluateExprVisitor : boost::static_visitor<std::variant<Value, RuntimeError>>
{
private:
  // NOTE: passing env as mutable reference does not meet the const requirement of operator()
  std::shared_ptr<Environment> env;

public:
  explicit EvaluateExprVisitor(std::shared_ptr<Environment> env_) : env(env_) {}

  std::variant<Value, RuntimeError> operator()(const Literal & literal)
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
      return std::string(literal.lexeme);
    }
    if (literal.type == TokenType::Number) {
      const double d = boost::lexical_cast<double>(literal.lexeme);
      if (literal.lexeme.find('.') != std::string::npos) {
        return d;
      }
      Value v = static_cast<int64_t>(d);
      return v;
    }

    // this is unreachable actually
    assert(false);  // LCOV_EXCL_LINE
    return expression::Nil{};
  }

  std::variant<Value, RuntimeError> operator()(const Unary & unary)
  {
    const auto right = boost::apply_visitor(*this, unary.expr);
    if (is_variant_v<RuntimeError>(right)) {
      return right;
    }
    const auto & right_value = as_variant<Value>(right);
    if (unary.op.type == TokenType::Minus) {
      if (!helper::is_numeric(right_value)) {
        // TODO(soblin): like -"123"
        return RuntimeError{RuntimeErrorKind::TypeError};
      }
      return helper::is_long(right_value) ? -as_variant<int64_t>(right_value)
                                          : -1.0 * as_variant<double>(right_value);
    }
    if (unary.op.type == TokenType::Bang) {
      return !is_truthy(right_value);
    }

    // this is unreachable actually
    assert(false);  // LCOV_EXCL_LINE
    return expression::Nil{};
  }

  std::variant<Value, RuntimeError> operator()(const Binary & binary)
  {
    const auto left_opt = boost::apply_visitor(*this, binary.left);
    if (is_variant_v<RuntimeError>(left_opt)) {
      return left_opt;
    }
    const auto right_opt = boost::apply_visitor(*this, binary.right);
    if (is_variant_v<RuntimeError>(right_opt)) {
      return right_opt;
    }
    const auto & left = as_variant<Value>(left_opt);
    const auto & right = as_variant<Value>(right_opt);

    // A * B
    if (binary.op.type == TokenType::Star) {
      if (!helper::is_numeric(left) || !helper::is_numeric(right)) {
        // TODO(soblin): like "123" * true
        return RuntimeError{RuntimeErrorKind::TypeError};
      }
      return apply_binary_op_scalar<std::multiplies>(left, right);
    }

    // A / B
    if (binary.op.type == TokenType::Slash) {
      if (!helper::is_numeric(left) || !helper::is_numeric(right)) {
        // TODO(soblin): like "123" * true
        return RuntimeError{RuntimeErrorKind::TypeError};
      }
      // TODO(soblin): ZeroDivisionError
      return apply_binary_op_scalar<std::divides>(left, right);
    }

    // A - B
    if (binary.op.type == TokenType::Minus) {
      if (!helper::is_numeric(left) || !helper::is_numeric(right)) {
        // TODO(soblin): like "123" * true
        return RuntimeError{RuntimeErrorKind::TypeError};
      }
      return apply_binary_op_scalar<std::minus>(left, right);
    }

    // A + B
    if (binary.op.type == TokenType::Plus) {
      if (helper::is_numeric(left) && helper::is_numeric(right)) {
        return apply_binary_op_scalar<std::plus>(left, right);
      }
      if (helper::is_str(left) && helper::is_str(right)) {
        return as_variant<std::string>(left) + as_variant<std::string>(right);
      }
      return RuntimeError{RuntimeErrorKind::TypeError};
    }

    // A > B
    if (binary.op.type == TokenType::Greater) {
      if (helper::is_numeric(left) && helper::is_numeric(right)) {
        return apply_binary_op_bool<std::greater>(left, right);
      }
      return RuntimeError{RuntimeErrorKind::TypeError};
    }

    // A >= B
    if (binary.op.type == TokenType::GreaterEqual) {
      if (helper::is_numeric(left) && helper::is_numeric(right)) {
        return apply_binary_op_bool<std::greater_equal>(left, right);
      }
      return RuntimeError{RuntimeErrorKind::TypeError};
    }

    // A < B
    if (binary.op.type == TokenType::Less) {
      if (helper::is_numeric(left) && helper::is_numeric(right)) {
        return apply_binary_op_bool<std::less>(left, right);
      }
      return RuntimeError{RuntimeErrorKind::TypeError};
    }

    // A <= B
    if (binary.op.type == TokenType::LessEqual) {
      if (helper::is_numeric(left) && helper::is_numeric(right)) {
        return apply_binary_op_bool<std::less_equal>(left, right);
      }
      return RuntimeError{RuntimeErrorKind::TypeError};
    }

    // A == B
    if (binary.op.type == TokenType::EqualEqual) {
      return is_equal(left, right);
    }

    // A != B
    if (binary.op.type == TokenType::BangEqual) {
      return !is_equal(left, right);
    }

    // A and B
    if (binary.op.type == TokenType::And) {
      // NOTE: https://en.wikipedia.org/wiki/Short-circuit_evaluation
      if (!is_truthy(left)) {
        return false;
      }
      return is_truthy(right);
    }

    // A or B
    if (binary.op.type == TokenType::Or) {
      // NOTE: https://en.wikipedia.org/wiki/Short-circuit_evaluation
      if (is_truthy(left)) {
        return true;
      }
      return is_truthy(right);
    }

    // this is unreachable actually
    assert(false);  // LCOV_EXCL_LINE
    return expression::Nil{};
  }

  std::variant<Value, RuntimeError> operator()(const Group & group)
  {
    return boost::apply_visitor(*this, group.expr);
  }

  std::variant<Value, RuntimeError> operator()(const Variable & variable)
  {
    return env->get(variable.name);
  }

  std::variant<Value, RuntimeError> operator()(const Assign & assign)
  {
    const auto rvalue_opt = boost::apply_visitor(*this, assign.expr);
    if (is_variant_v<RuntimeError>(rvalue_opt)) {
      return as_variant<RuntimeError>(rvalue_opt);
    }
    const auto & rvalue = as_variant<Value>(rvalue_opt);
    env->define(assign.name, rvalue);
    return rvalue;
  }
};

auto Interpreter::evaluate_expr(const Expr & expr) -> std::variant<Value, RuntimeError>
{
  auto evaluator = EvaluateExprVisitor(env_);
  return boost::apply_visitor(evaluator, expr);
}

auto Interpreter::get_variable(const Token & token) const -> std::optional<Value>
{
  if (const auto it = env_->get(token); is_variant_v<Value>(it) == true) {
    return as_variant<Value>(it);
  }
  return std::nullopt;
}

}  // namespace interpreter
}  // namespace lox
