#pragma once

#include <cpplox/environment.hpp>
#include <cpplox/error.hpp>
#include <cpplox/expression.hpp>
#include <cpplox/statement.hpp>

#include <optional>
#include <vector>

namespace lox
{

inline namespace interpreter
{

class Interpreter
{
public:
  auto execute(const std::vector<Stmt> & program) -> std::optional<RuntimeError>;
};

auto evaluate_expr(const Expr & expr) -> std::variant<Value, RuntimeError>;

}  // namespace interpreter
}  // namespace lox
