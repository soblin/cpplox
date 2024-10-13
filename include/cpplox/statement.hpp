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

struct WhileStmt;

struct ForStmt;

using Stmt = boost::variant<
  ExprStmt, PrintStmt, boost::recursive_wrapper<Block>, boost::recursive_wrapper<IfBlock>,
  boost::recursive_wrapper<WhileStmt>, boost::recursive_wrapper<ForStmt>>;

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

struct WhileStmt
{
  const Expr cond;
  const std::vector<Declaration> body;
};

struct ForStmt
{
  const std::optional<std::variant<VarDecl, ExprStmt>> init_stmt;
  const std::optional<Expr> cond;
  const std::optional<Expr> next;
  const std::vector<Declaration> declarations;
};

using Program = std::vector<Declaration>;

}  // namespace stmt
}  // namespace lox
