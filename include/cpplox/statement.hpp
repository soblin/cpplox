#pragma once
#include <cpplox/expression.hpp>

#include <boost/variant/recursive_variant.hpp>

#include <optional>
#include <unordered_map>
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

struct BreakStmt;

struct ContinueStmt;

struct ReturnStmt
{
  const std::optional<Expr> expr;
};

using Stmt = boost::variant<
  ExprStmt, PrintStmt, boost::recursive_wrapper<Block>, boost::recursive_wrapper<IfBlock>,
  boost::recursive_wrapper<WhileStmt>, boost::recursive_wrapper<ForStmt>, BreakStmt, ContinueStmt,
  ReturnStmt>;

struct VarDecl
{
  const Token name;
  const std::optional<Expr> initializer;
};

struct FuncDecl;

struct ClassDecl;

using Declaration = boost::variant<
  VarDecl, Stmt, boost::recursive_wrapper<FuncDecl>, boost::recursive_wrapper<ClassDecl>>;

struct Block
{
  std::vector<Declaration> declarations;
};

struct FuncDecl
{
  const Token name;
  const Tokens parameters;
  const Block body;
};

struct ClassDecl
{
  const Token name;
  const std::unordered_map<std::string_view, FuncDecl> methods;
};

struct BranchClause
{
  const std::optional<VarDecl> declaration;
  const Expr cond;
  const Block body;
};

struct IfBlock
{
  const BranchClause if_clause;
  const std::vector<BranchClause> elseif_clauses;
  const std::optional<Block> else_body;
};

struct WhileStmt
{
  const Token while_token;
  const Expr cond;
  const Block body;
};

struct ForStmt
{
  const Token for_token;
  const std::optional<std::variant<VarDecl, ExprStmt>> init_stmt;
  const std::optional<Expr> cond;
  const std::optional<Expr> next;
  const Block body;
};

struct BreakStmt
{
};

struct ContinueStmt
{
};

using Program = std::vector<Declaration>;

}  // namespace stmt
}  // namespace lox
