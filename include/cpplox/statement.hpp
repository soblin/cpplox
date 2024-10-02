#pragma once
#include <cpplox/expression.hpp>

#include <optional>
#include <vector>

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

struct VarDecl
{
  const Token name;
  const std::optional<Expr> initializer;
};

using Stmt = std::variant<ExprStmt, PrintStmt>;

using Declaration = std::variant<VarDecl, Stmt>;

using Program = std::vector<Declaration>;

}  // namespace stmt
}  // namespace lox
