#pragma once

#include <cpplox/environment.hpp>
#include <cpplox/error.hpp>
#include <cpplox/expression.hpp>
#include <cpplox/statement.hpp>

#include <memory>
#include <optional>

namespace lox
{

inline namespace interpreter
{

class Interpreter
{
public:
  Interpreter() = default;

  /**
   * @brief execute the given program
   */
  [[nodiscard]] auto execute(const Program & program) -> std::optional<RuntimeError>;

  /**
   * @brief evaluate the given expression
   */
  auto evaluate_expr(const Expr & expr) -> std::variant<Value, RuntimeError>;

  auto get_variable(const Token & token) const -> std::optional<Value>;

private:
  std::shared_ptr<Environment> env_{std::make_shared<Environment>()};

  /**
   * @brief execute the given declaration
   */
  auto execute_declaration(const Declaration & declaration) -> std::optional<RuntimeError>;
};

namespace impl
{

class EvaluateExprVisitor : boost::static_visitor<std::variant<Value, RuntimeError>>
{
private:
  // NOTE: passing env as mutable reference does not meet the const requirement of operator()
  std::shared_ptr<Environment> env;

public:
  explicit EvaluateExprVisitor(std::shared_ptr<Environment> env_) : env(env_) {}

  std::variant<Value, RuntimeError> operator()(const Literal & literal);

  std::variant<Value, RuntimeError> operator()(const Unary & unary);

  std::variant<Value, RuntimeError> operator()(const Binary & binary);

  std::variant<Value, RuntimeError> operator()(const Group & group);

  std::variant<Value, RuntimeError> operator()(const Variable & variable);

  std::variant<Value, RuntimeError> operator()(const Assign & assign);
};

auto evaluate_expr_impl(const Expr & expr, std::shared_ptr<Environment> env)
  -> std::variant<Value, RuntimeError>;

class ExecuteStmtVisitor : boost::static_visitor<std::optional<RuntimeError>>
{
private:
  std::shared_ptr<Environment> env;

public:
  explicit ExecuteStmtVisitor(std::shared_ptr<Environment> env) : env(env) {}

  std::optional<RuntimeError> operator()(const ExprStmt & stmt);

  std::optional<RuntimeError> operator()(const PrintStmt & stmt);

  std::optional<RuntimeError> operator()(const Block & block);
};

auto execute_stmt_impl(const Stmt & stmt, std::shared_ptr<Environment> env)
  -> std::optional<RuntimeError>;

class ExecuteDeclarationVisitor : boost::static_visitor<std::optional<RuntimeError>>
{
private:
  std::shared_ptr<Environment> env;

public:
  explicit ExecuteDeclarationVisitor(std::shared_ptr<Environment> env) : env(env) {}

  std::optional<RuntimeError> operator()(const VarDecl & decl);

  std::optional<RuntimeError> operator()(const Stmt & stmt);
};

}  // namespace impl
}  // namespace interpreter
}  // namespace lox
