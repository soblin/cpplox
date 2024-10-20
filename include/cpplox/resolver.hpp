#pragma once

#include <cpplox/error.hpp>
#include <cpplox/expression.hpp>
#include <cpplox/statement.hpp>

#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/variant/recursive_variant.hpp>

#include <deque>
#include <string>

namespace lox
{

inline namespace resolver
{

using ScopeChain = std::deque<boost::unordered_flat_map<std::string, bool>>;
using ScopeLookup = boost::unordered_flat_map<Token, size_t>;

class StmtResolver : boost::static_visitor<std::optional<CompileError>>
{
public:
  StmtResolver(ScopeChain & scopes, ScopeLookup & lookup) : scopes(scopes), lookup(lookup) {}

  std::optional<CompileError> operator()(const ExprStmt & stmt);

  std::optional<CompileError> operator()(const PrintStmt & stmt);

  std::optional<CompileError> operator()(const Block & block);

  std::optional<CompileError> operator()(const IfBlock & stmt);

  std::optional<CompileError> operator()(const WhileStmt & stmt);

  std::optional<CompileError> operator()(const ForStmt & stmt);

  std::optional<CompileError> operator()(const BreakStmt & stmt);

  std::optional<CompileError> operator()(const ContinueStmt & stmt);

  std::optional<CompileError> operator()(const ReturnStmt & stmt);

private:
  ScopeChain & scopes;
  ScopeLookup & lookup;

  void begin_scope();
  void end_scope();
};

class DeclResolver : boost::static_visitor<std::optional<CompileError>>
{
public:
  DeclResolver(ScopeChain & scopes, ScopeLookup & lookup) : scopes(scopes), lookup(lookup) {}

  std::optional<CompileError> operator()(const VarDecl & var_decl);

  std::optional<CompileError> operator()(const Stmt & stmt);

  std::optional<CompileError> operator()(const FuncDecl & func_decl);

private:
  ScopeChain & scopes;
  ScopeLookup & lookup;

  void begin_scope();
  void end_scope();
  void declare(const Token & name);
  void define(const Token & name);
};

class ExprResolver : boost::static_visitor<std::optional<CompileError>>
{
public:
  ExprResolver(ScopeChain & scopes, ScopeLookup & lookup) : scopes(scopes), lookup(lookup) {}

  std::optional<CompileError> operator()(const Literal & literal);

  std::optional<CompileError> operator()(const Unary & unary);

  std::optional<CompileError> operator()(const Binary & binary);

  std::optional<CompileError> operator()(const Group & group);

  std::optional<CompileError> operator()(const Variable & expr);

  std::optional<CompileError> operator()(const Assign & assign);

  std::optional<CompileError> operator()(const Logical & logical);

  std::optional<CompileError> operator()(const Call & call);

private:
  ScopeChain & scopes;
  ScopeLookup & lookup;

  void resolve_local(const Token & name);
};

}  // namespace resolver
}  // namespace lox
