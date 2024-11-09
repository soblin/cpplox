#pragma once

#include <cpplox/control_flow.hpp>
#include <cpplox/environment.hpp>
#include <cpplox/error.hpp>
#include <cpplox/expression.hpp>
#include <cpplox/resolver.hpp>
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

  [[nodiscard]] auto resolve(const Program & program) -> std::optional<CompileError>;

  auto print_resolve(const Program & program) -> void;

  /**
   * @brief evaluate the given expression
   */
  auto evaluate_expr(const Expr & expr) -> std::variant<Value, RuntimeError>;

  auto get_variable(const Token & token) const -> std::optional<Value>;

private:
  std::shared_ptr<Environment> global_env_;
  ScopeLookup lookup_;

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
  const ScopeLookup & lookup_;
  // NOTE: passing env as mutable reference does not meet the const requirement of operator()
  std::shared_ptr<Environment> env;
  // if the expression contained function call, the function must not use `env`, because function
  // must not refer to the parameters defined out of its scope. it is only allowed to access to the
  // scope enclosing global scope + its parameters.
  std::shared_ptr<Environment> global_env;

public:
  explicit EvaluateExprVisitor(
    const ScopeLookup & lookup, std::shared_ptr<Environment> env_,
    std::shared_ptr<Environment> global_env_)
  : lookup_(lookup), env(env_), global_env(global_env_)
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

  std::variant<Value, RuntimeError> operator()(const ReadProperty & property);

  std::variant<Value, RuntimeError> operator()(const SetProperty & property);
};

auto evaluate_expr_impl(
  const Expr & expr, const ScopeLookup & lookup, std::shared_ptr<Environment> env,
  std::shared_ptr<Environment> global_env) -> std::variant<Value, RuntimeError>;

class ExecuteStmtVisitor : boost::static_visitor<std::optional<RuntimeError>>
{
private:
  const ScopeLookup & lookup_;
  std::shared_ptr<Environment> env;
  std::shared_ptr<Environment> global_env;
  std::optional<ControlFlowKind> & procedure;

public:
  explicit ExecuteStmtVisitor(
    const ScopeLookup & lookup, std::shared_ptr<Environment> env,
    std::shared_ptr<Environment> global_env, std::optional<ControlFlowKind> & proc)
  : lookup_(lookup), env(env), global_env(global_env), procedure(proc)
  {
    assert(!procedure);
  }

  std::variant<bool, RuntimeError> execute_branch_clause(
    const BranchClause & clause, std::shared_ptr<Environment> env);

  /**
   * @brief execute <expr_statement>, so environment is updated
   * @post procedure remains null
   */
  std::optional<RuntimeError> operator()(const ExprStmt & stmt);

  /**
   * @brief execute <print_statement>
   * @post procedure remains null
   */
  std::optional<RuntimeError> operator()(const PrintStmt & stmt);

  /**
   * @brief execute <expr_statement>, so environment is updated
   * @post procedure is activated if break/continue is called directly
   */
  std::optional<RuntimeError> operator()(const Block & block);

  /**
   * @brief execute <if_block>, so environment is updated
   * @post procedure is activated if break/continue is called directly
   */
  std::optional<RuntimeError> operator()(const IfBlock & if_block);

  /**
   * @brief execute <while_statement>
   * @post procedure remains null
   */
  std::optional<RuntimeError> operator()(const WhileStmt & while_stmt);

  /**
   * @brief execute <for_statement>
   * @post procedure remains null
   */
  std::optional<RuntimeError> operator()(const ForStmt & for_stmt);

  /**
   * @brief almost do nothing
   * @post procedure gets ControlFlowKind::Break
   */
  std::optional<RuntimeError> operator()(const BreakStmt & break_stmt);

  /**
   * @brief almost do nothing
   * @post procedure gets ControlFlowKind::Continue
   */
  std::optional<RuntimeError> operator()(const ContinueStmt & continue_stmt);

  /**
   * @brief return value if provided
   * @post procedure may turn ControlFlow::Return
   */
  std::optional<RuntimeError> operator()(const ReturnStmt & return_stmt);
};

auto execute_stmt_impl(
  const Stmt & stmt, const ScopeLookup & lookup, std::shared_ptr<Environment> env,
  std::shared_ptr<Environment> global_env,
  std::optional<ControlFlowKind> & procedure) -> std::optional<RuntimeError>;

class ExecuteDeclarationVisitor : boost::static_visitor<std::optional<RuntimeError>>
{
private:
  const ScopeLookup & lookup_;
  std::shared_ptr<Environment> env;
  std::shared_ptr<Environment> global_env;
  std::optional<ControlFlowKind> & procedure;

public:
  explicit ExecuteDeclarationVisitor(
    const ScopeLookup & lookup, std::shared_ptr<Environment> env,
    std::shared_ptr<Environment> global_env, std::optional<ControlFlowKind> & proc)
  : lookup_(lookup), env(env), global_env(global_env), procedure(proc)
  {
  }

  std::optional<RuntimeError> operator()(const VarDecl & decl);

  std::optional<RuntimeError> operator()(const Stmt & stmt);

  std::optional<RuntimeError> operator()(const FuncDecl & func_decl);

  std::optional<RuntimeError> operator()(const ClassDecl & class_decl);
};

}  // namespace impl
}  // namespace interpreter
}  // namespace lox
