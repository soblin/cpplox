#pragma once

#include <cpplox/environment.hpp>
#include <cpplox/error.hpp>
#include <cpplox/expression.hpp>
#include <cpplox/statement.hpp>

#include <memory>
#include <optional>

namespace lox
{

inline namespace interpreter
{

class Interpreter
{
public:
  Interpreter() = default;

  /**
   * @brief execute the given program
   */
  auto execute(const Program & program) -> std::optional<RuntimeError>;

  /**
   * @brief evaluate the given expression
   */
  auto evaluate_expr(const Expr & expr) -> std::variant<Value, RuntimeError>;

  auto get_variable(const Token & token) const -> std::optional<Value>;

private:
  std::shared_ptr<Environment> env_{std::make_shared<Environment>()};
};

}  // namespace interpreter
}  // namespace lox
