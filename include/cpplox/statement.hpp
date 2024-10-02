#pragma once
#include <cpplox/expression.hpp>

#include <boost/variant/recursive_variant.hpp>

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

struct Block;
using Stmt = boost::variant<ExprStmt, PrintStmt, boost::recursive_wrapper<Block>>;

using Declaration = boost::variant<VarDecl, boost::recursive_wrapper<Stmt>>;

struct Block
{
  std::vector<Declaration> declarations;
};

using Program = std::vector<Declaration>;

}  // namespace stmt
}  // namespace lox
