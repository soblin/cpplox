#pragma once
#include <cpplox/expression.hpp>

#include <optional>

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

struct VarDeclStmt
{
  const Token name;
  const std::optional<Expr> initializer;
};

using Stmt = std::variant<ExprStmt, PrintStmt, VarDeclStmt>;

}  // namespace stmt
}  // namespace lox
