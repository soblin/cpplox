#pragma once

#include <cpplox/environment.hpp>
#include <cpplox/error.hpp>
#include <cpplox/expression.hpp>
#include <cpplox/statement.hpp>

#include <memory>
#include <optional>
#include <vector>

namespace lox
{

inline namespace interpreter
{

class Interpreter
{
public:
  Interpreter() = default;
  auto execute(const std::vector<Stmt> & program) -> std::optional<RuntimeError>;
  auto evaluate_expr(const Expr & expr) -> std::variant<Value, RuntimeError>;

private:
  std::shared_ptr<Environment> env_{std::make_shared<Environment>()};
};

}  // namespace interpreter
}  // namespace lox
