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
  Interpreter() { global_env_ = std::make_shared<Environment>(); }

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
  std::shared_ptr<Environment> global_env_;

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
  // if the expression contained function call, the function must not use `env`, because function
  // must not refer to the parameters defined out of its scope. it is only allowed to access to the
  // scope enclosing global scope + its parameters.
  std::shared_ptr<Environment> global_env;

public:
  explicit EvaluateExprVisitor(
    std::shared_ptr<Environment> env_, std::shared_ptr<Environment> global_env_)
  : env(env_), global_env(global_env_)
  {
  }

  std::variant<Value, RuntimeError> operator()(const Literal & literal);

  std::variant<Value, RuntimeError> operator()(const Unary & unary);

  std::variant<Value, RuntimeError> operator()(const Binary & binary);

  std::variant<Value, RuntimeError> operator()(const Group & group);

  std::variant<Value, RuntimeError> operator()(const Variable & variable);

  std::variant<Value, RuntimeError> operator()(const Assign & assign);

  std::variant<Value, RuntimeError> operator()(const Logical & logical);

  std::variant<Value, RuntimeError> operator()(const Call & call);
};

auto evaluate_expr_impl(
  const Expr & expr, std::shared_ptr<Environment> env,
  std::shared_ptr<Environment> global_env) -> std::variant<Value, RuntimeError>;

class ExecuteStmtVisitor : boost::static_visitor<std::optional<RuntimeError>>
{
private:
  std::shared_ptr<Environment> env;
  std::shared_ptr<Environment> global_env;
  std::optional<PseudoSignalKind> & signal;

public:
  explicit ExecuteStmtVisitor(
    std::shared_ptr<Environment> env, std::shared_ptr<Environment> global_env,
    std::optional<PseudoSignalKind> & sig)
  : env(env), global_env(global_env), signal(sig)
  {
    assert(!signal);
  }

  std::variant<bool, RuntimeError> execute_branch_clause(
    const BranchClause & clause, std::shared_ptr<Environment> env);

  /**
   * @brief execute <expr_statement>, so environment is updated
   * @post signal remains null
   */
  std::optional<RuntimeError> operator()(const ExprStmt & stmt);

  /**
   * @brief execute <print_statement>
   * @post signal remains null
   */
  std::optional<RuntimeError> operator()(const PrintStmt & stmt);

  /**
   * @brief execute <expr_statement>, so environment is updated
   * @post signal is activated if break/continue is called directly
   */
  std::optional<RuntimeError> operator()(const Block & block);

  /**
   * @brief execute <if_block>, so environment is updated
   * @post signal is activated if break/continue is called directly
   */
  std::optional<RuntimeError> operator()(const IfBlock & if_block);

  /**
   * @brief execute <while_statement>
   * @post signal remains null
   */
  std::optional<RuntimeError> operator()(const WhileStmt & while_stmt);

  /**
   * @brief execute <for_statement>
   * @post signal remains null
   */
  std::optional<RuntimeError> operator()(const ForStmt & for_stmt);

  /**
   * @brief almost do nothing
   * @post signal gets PseudoErrorKind::Break
   */
  std::optional<RuntimeError> operator()(const BreakStmt & break_stmt);

  /**
   * @brief almost do nothing
   * @post signal gets PseudoErrorKind::Continue
   */
  std::optional<RuntimeError> operator()(const ContinueStmt & continue_stmt);
};

auto execute_stmt_impl(
  const Stmt & stmt, std::shared_ptr<Environment> env, std::shared_ptr<Environment> global_env,
  std::optional<PseudoSignalKind> & signal) -> std::optional<RuntimeError>;

class ExecuteDeclarationVisitor : boost::static_visitor<std::optional<RuntimeError>>
{
private:
  std::shared_ptr<Environment> env;
  std::shared_ptr<Environment> global_env;
  std::optional<PseudoSignalKind> & signal;

public:
  explicit ExecuteDeclarationVisitor(
    std::shared_ptr<Environment> env, std::shared_ptr<Environment> global_env,
    std::optional<PseudoSignalKind> & sig)
  : env(env), global_env(global_env), signal(sig)
  {
  }

  std::optional<RuntimeError> operator()(const VarDecl & decl);

  std::optional<RuntimeError> operator()(const Stmt & stmt);

  std::optional<RuntimeError> operator()(const FuncDecl & func_decl);
};

}  // namespace impl
}  // namespace interpreter
}  // namespace lox
