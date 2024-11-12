#pragma once

#include <cpplox/error.hpp>
#include <cpplox/expression.hpp>
#include <cpplox/resolver.hpp>

#include <iosfwd>
#include <sstream>
#include <string>

namespace lox
{

inline namespace debug
{
/**
 * @brief convert Binary('1', '+', '2') to list-style "(+ 1 2)"
 */
auto to_lisp_repr(const Expr & expr) -> std::string;

static constexpr const char * Bold = "\033[1m";
static constexpr const char * Italic = "\033[3m";
static constexpr const char * Thin = "\033[2m";
static constexpr const char * Underline = "\033[4m";

static constexpr const char * Red = "\033[31m";
static constexpr const char * Green = "\033[32m";
static constexpr const char * Yellow = "\033[33m";
static constexpr const char * Blue = "\033[34m";
static constexpr const char * Magenta = "\033[35m";
static constexpr const char * Cyan = "\033[36m";

static constexpr const char * Reset = "\033[0m";

class PrintResolveExprVisitor : boost::static_visitor<void>
{
public:
  std::stringstream ss;

  explicit PrintResolveExprVisitor(const ScopeLookup & lookup) : lookup(lookup) {}

  void operator()(const Literal & expr);

  void operator()(const Unary & expr);

  void operator()(const Binary & expr);

  void operator()(const Group & expr);

  void operator()(const Variable & expr);

  void operator()(const Assign & expr);

  void operator()(const Logical & expr);

  void operator()(const Call & expr);

  void operator()(const ReadProperty & expr);

  void operator()(const SetProperty & expr);

private:
  const ScopeLookup & lookup;
};

class PrintResolveStmtVisitor : boost::static_visitor<void>
{
public:
  std::stringstream ss;

  PrintResolveStmtVisitor(const size_t offset, const ScopeLookup & lookup)
  : offset(offset), lookup(lookup)
  {
  }

  void operator()(const ExprStmt & stmt);

  void operator()(const PrintStmt & stmt);

  void operator()(const Block & stmt);

  void operator()(const IfBlock & stmt);

  void operator()(const WhileStmt & stmt);

  void operator()(const ForStmt & stmt);

  void operator()(const BreakStmt & stmt);

  void operator()(const ContinueStmt & stmt);

  void operator()(const ReturnStmt & stmt);

private:
  const size_t offset;
  const ScopeLookup & lookup;
  const size_t skip{4};
};

class PrintResolveDeclVisitor : boost::static_visitor<void>
{
public:
  std::stringstream ss;

  PrintResolveDeclVisitor(const size_t offset, const ScopeLookup & lookup)
  : offset(offset), lookup(lookup)
  {
  }

  void operator()(const VarDecl & var_decl);

  void operator()(const Stmt & stmt);

  void operator()(const FuncDecl & func_decl);

  void operator()(const ClassDecl & class_decl);

private:
  const size_t offset;
  const ScopeLookup & lookup;
  const size_t skip{4};
};

class StringifyExprVisitor : boost::static_visitor<std::string>
{
public:
  std::string operator()(const Nil & expr) { return "nil"; }

  std::string operator()(const bool & expr) { return expr ? "true" : "false"; }

  std::string operator()(const int64_t & expr) { return std::to_string(expr); }

  std::string operator()(const double & expr) { return std::to_string(expr); }

  std::string operator()(const std::string & expr) { return expr; }

  std::string operator()(const Callable & expr)
  {
    return "<fn " + std::string(expr.definition->name.lexeme) + " >";
  }

  std::string operator()(const Class & expr)
  {
    return "<class definition " + std::string(expr->definition->name.lexeme) + " >";
  }

  std::string operator()(const Instance & expr)
  {
    return "<instance " + std::string(expr.cls->definition->name.lexeme) + ">";
  }
};

auto stringify(const Value & value) -> std::string;

}  // namespace debug
}  // namespace lox

namespace lox
{
inline namespace expression
{
std::ostream & operator<<(std::ostream & os, const Value & value);
}  // namespace expression
}  // namespace lox
