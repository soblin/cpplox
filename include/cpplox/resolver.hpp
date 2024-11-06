#pragma once

#include <cpplox/error.hpp>
#include <cpplox/expression.hpp>
#include <cpplox/statement.hpp>

#include <boost/variant/recursive_variant.hpp>

#include <deque>
#include <string>
#include <unordered_map>

namespace std
{
template <>
struct hash<lox::token::Token>
{
  size_t operator()(const lox::token::Token & token) const
  {
    size_t h1 = std::hash<lox::token::TokenType>()(token.type);
    size_t h2 = std::hash<std::string_view>()(token.lexeme);
    size_t h3 = std::hash<size_t>()(token.start_index);
    return h1 ^ (h2 << 1) ^ (h3 << 2);
  }
};
}  // namespace std

namespace lox
{

inline namespace resolver
{

using ScopeChain = std::deque<std::unordered_map<std::string_view, bool>>;
using ScopeLookup = std::unordered_map<Token, size_t>;

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

  std::optional<CompileError> operator()(const ClassDecl & class_decl);

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

  std::optional<CompileError> operator()(const ReadProperty & property);

private:
  ScopeChain & scopes;
  ScopeLookup & lookup;

  void resolve_local(const Token & name);
};

}  // namespace resolver
}  // namespace lox
