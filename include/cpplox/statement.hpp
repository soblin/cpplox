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

struct Block;

struct IfBlock;

using Stmt = boost::variant<
  ExprStmt, PrintStmt, boost::recursive_wrapper<Block>, boost::recursive_wrapper<IfBlock>>;

struct VarDecl
{
  const Token name;
  const std::optional<Expr> initializer;
};

using Declaration = boost::variant<VarDecl, Stmt>;

struct Block
{
  std::vector<Declaration> declarations;
};

struct BranchClause
{
  const std::optional<VarDecl> declaration;
  const Expr cond;
  const std::vector<Declaration> body;
};

struct IfBlock
{
  const BranchClause if_clause;
  const std::vector<BranchClause> elseif_clauses;
  const std::optional<std::vector<Declaration>> else_body;
};

using Program = std::vector<Declaration>;

}  // namespace stmt
}  // namespace lox
