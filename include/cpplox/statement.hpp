#pragma once
#include <cpplox/expression.hpp>

namespace lox
{

inline namespace stmt
{

struct ExprStmt
{
  const Expr expression;
};

struct PrintStmt
{
  const Expr expression;
};

using Stmt = std::variant<ExprStmt, PrintStmt>;

}  // namespace stmt
}  // namespace lox
