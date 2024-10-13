#include <cpplox/environment.hpp>
#include <cpplox/interpreter.hpp>

#include <boost/lexical_cast.hpp>

#include <functional>
#include <iostream>

namespace lox
{

inline namespace interpreter
{

namespace
{
// LCOV_EXCL_START
auto stringify = [](const Value & value) -> std::string {
  return std::visit(
    visit_variant{
      [](const Nil & nil) -> std::string { return "nil"; },
      [](const bool & boolean) -> std::string { return boolean ? "true" : "false"; },
      [](const int64_t & i) -> std::string { return std::to_string(i); },
      [](const double & d) -> std::string { return std::to_string(d); },
      [](const std::string & str) -> std::string { return str; },
    },
    value);
};
// LCOV_EXCL_STOP
}  // namespace

auto Interpreter::evaluate_expr(const Expr & expr) -> std::variant<Value, RuntimeError>
{
  return impl::evaluate_expr_impl(expr, env_);
}  // LCOV_EXCL_LINE

auto Interpreter::execute_declaration(const Declaration & declaration)
  -> std::optional<RuntimeError>
{
  impl::ExecuteDeclarationVisitor executor(env_);
  return boost::apply_visitor(executor, declaration);
}

auto Interpreter::execute(const Program & program) -> std::optional<RuntimeError>
{
  for (const auto & declaration : program) {
    const std::optional<RuntimeError> result = execute_declaration(declaration);
    if (result) {
      return result.value();
    }
  }
  return std::nullopt;
}

auto Interpreter::get_variable(const Token & token) const -> std::optional<Value>
{
  if (const auto it = env_->get(token); is_variant_v<Value>(it) == true) {
    return as_variant<Value>(it);
  }
  return std::nullopt;
}

namespace impl
{
template <template <typename> class F>
auto apply_binary_op_scalar(const Value & left_numeric, const Value & right_numeric) -> Value
{
  if (helper::is_long(left_numeric)) {
    return helper::is_long(right_numeric)
             ? Value{static_cast<int64_t>(F<int64_t>()(
                 as_variant<int64_t>(left_numeric), as_variant<int64_t>(right_numeric)))}
             : Value{static_cast<double>(F<double>()(
                 as_variant<int64_t>(left_numeric), as_variant<double>(right_numeric)))};
  }
  return helper::is_long(right_numeric)
           ? Value{static_cast<double>(
               F<double>()(as_variant<double>(left_numeric), as_variant<int64_t>(right_numeric)))}
           : Value{static_cast<double>(
               F<double>()(as_variant<double>(left_numeric), as_variant<double>(right_numeric)))};
}

template <template <typename> class F>
auto apply_binary_op_bool(const Value & left_numeric, const Value & right_numeric) -> Value
{
  if (helper::is_long(left_numeric)) {
    return helper::is_long(right_numeric)
             ? Value{static_cast<bool>(F<int64_t>()(
                 as_variant<int64_t>(left_numeric), as_variant<int64_t>(right_numeric)))}
             : Value{static_cast<bool>(F<double>()(
                 as_variant<int64_t>(left_numeric), as_variant<double>(right_numeric)))};
  }
  return helper::is_long(right_numeric)
           ? Value{static_cast<bool>(
               F<double>()(as_variant<double>(left_numeric), as_variant<int64_t>(right_numeric)))}
           : Value{static_cast<bool>(
               F<double>()(as_variant<double>(left_numeric), as_variant<double>(right_numeric)))};
}

std::variant<Value, RuntimeError> EvaluateExprVisitor::operator()(const Literal & literal)
{
  if (literal.type == TokenType::Nil) {
    return expression::Nil{};
  }
  if (literal.type == TokenType::False) {
    return false;
  }
  if (literal.type == TokenType::True) {
    return true;
  }
  if (literal.type == TokenType::String) {
    return std::string(literal.lexeme);
  }
  if (literal.type == TokenType::Number) {
    const double d = boost::lexical_cast<double>(literal.lexeme);
    if (literal.lexeme.find('.') != std::string::npos) {
      return d;
    }
    Value v = static_cast<int64_t>(d);
    return v;
  }

  // this is unreachable actually
  assert(false);  // LCOV_EXCL_LINE
  return expression::Nil{};
}

std::variant<Value, RuntimeError> EvaluateExprVisitor::operator()(const Unary & unary)
{
  const auto right = boost::apply_visitor(*this, unary.expr);
  if (is_variant_v<RuntimeError>(right)) {
    return right;
  }
  const auto & right_value = as_variant<Value>(right);
  if (unary.op.type == TokenType::Minus) {
    if (!helper::is_numeric(right_value)) {
      return TypeError{unary.op, unary.expr};
    }
    return helper::is_long(right_value) ? -as_variant<int64_t>(right_value)
                                        : -1.0 * as_variant<double>(right_value);
  }
  if (unary.op.type == TokenType::Bang) {
    return !is_truthy(right_value);
  }

  // this is unreachable actually
  assert(false);  // LCOV_EXCL_LINE
  return expression::Nil{};
}

std::variant<Value, RuntimeError> EvaluateExprVisitor::operator()(const Binary & binary)
{
  const auto left_opt = boost::apply_visitor(*this, binary.left);
  if (is_variant_v<RuntimeError>(left_opt)) {
    return left_opt;
  }
  const auto right_opt = boost::apply_visitor(*this, binary.right);
  if (is_variant_v<RuntimeError>(right_opt)) {
    return right_opt;
  }
  const auto & left = as_variant<Value>(left_opt);
  const auto & right = as_variant<Value>(right_opt);

  // A * B
  if (binary.op.type == TokenType::Star) {
    if (!helper::is_numeric(left) || !helper::is_numeric(right)) {
      return TypeError{binary.op, binary};
    }
    return apply_binary_op_scalar<std::multiplies>(left, right);
  }

  // A / B
  if (binary.op.type == TokenType::Slash) {
    if (!helper::is_numeric(left) || !helper::is_numeric(right)) {
      return TypeError{binary.op, binary};
    }
    // TODO(soblin): ZeroDivisionError
    return apply_binary_op_scalar<std::divides>(left, right);
  }

  // A - B
  if (binary.op.type == TokenType::Minus) {
    if (!helper::is_numeric(left) || !helper::is_numeric(right)) {
      return TypeError{binary.op, binary};
    }
    return apply_binary_op_scalar<std::minus>(left, right);
  }

  // A + B
  if (binary.op.type == TokenType::Plus) {
    if (helper::is_numeric(left) && helper::is_numeric(right)) {
      return apply_binary_op_scalar<std::plus>(left, right);
    }
    if (helper::is_str(left) && helper::is_str(right)) {
      return as_variant<std::string>(left) + as_variant<std::string>(right);
    }
    return TypeError{binary.op, binary};
  }

  // A > B
  if (binary.op.type == TokenType::Greater) {
    if (helper::is_numeric(left) && helper::is_numeric(right)) {
      return apply_binary_op_bool<std::greater>(left, right);
    }
    return TypeError{binary.op, binary};
  }

  // A >= B
  if (binary.op.type == TokenType::GreaterEqual) {
    if (helper::is_numeric(left) && helper::is_numeric(right)) {
      return apply_binary_op_bool<std::greater_equal>(left, right);
    }
    return TypeError{binary.op, binary};
  }

  // A < B
  if (binary.op.type == TokenType::Less) {
    if (helper::is_numeric(left) && helper::is_numeric(right)) {
      return apply_binary_op_bool<std::less>(left, right);
    }
    return TypeError{binary.op, binary};
  }

  // A <= B
  if (binary.op.type == TokenType::LessEqual) {
    if (helper::is_numeric(left) && helper::is_numeric(right)) {
      return apply_binary_op_bool<std::less_equal>(left, right);
    }
    return TypeError{binary.op, binary};
  }

  // A == B
  if (binary.op.type == TokenType::EqualEqual) {
    return is_equal(left, right);
  }

  // A != B
  if (binary.op.type == TokenType::BangEqual) {
    return !is_equal(left, right);
  }

  // this is unreachable actually
  assert(false);  // LCOV_EXCL_LINE
  return expression::Nil{};
}

std::variant<Value, RuntimeError> EvaluateExprVisitor::operator()(const Group & group)
{
  return boost::apply_visitor(*this, group.expr);
}

std::variant<Value, RuntimeError> EvaluateExprVisitor::operator()(const Variable & variable)
{
  return env->get(variable.name);
}

std::variant<Value, RuntimeError> EvaluateExprVisitor::operator()(const Assign & assign)
{
  const auto rvalue_opt = boost::apply_visitor(*this, assign.expr);
  if (is_variant_v<RuntimeError>(rvalue_opt)) {
    return as_variant<RuntimeError>(rvalue_opt);
  }
  const auto & rvalue = as_variant<Value>(rvalue_opt);
  const auto assign_err = env->assign(assign.name, rvalue);
  if (assign_err) {
    // NOTE: returned value from env does not contain expr information
    return UndefinedVariableError{assign.name, assign.expr};
  }
  return rvalue;
}

std::variant<Value, RuntimeError> EvaluateExprVisitor::operator()(const Logical & logical)
{
  const auto left_value_opt = boost::apply_visitor(*this, logical.left);
  if (is_variant_v<RuntimeError>(left_value_opt)) {
    return as_variant<RuntimeError>(left_value_opt);
  }
  const auto & left_value = as_variant<Value>(left_value_opt);
  const auto is_left_true = is_truthy(left_value);

  if (!is_left_true && logical.op.type == TokenType::And) {
    return false;
  }
  if (is_left_true && logical.op.type == TokenType::Or) {
    return true;
  }

  const auto right_value_opt = boost::apply_visitor(*this, logical.right);
  if (is_variant_v<RuntimeError>(right_value_opt)) {
    return as_variant<RuntimeError>(right_value_opt);
  }
  const auto & right_value = as_variant<Value>(right_value_opt);
  const auto is_right_true = is_truthy(right_value);

  if (logical.op.type == TokenType::And) {
    return is_left_true and is_right_true;
  }
  if (logical.op.type == TokenType::Or) {
    return is_left_true or is_right_true;
  }

  // this is unreachable
  assert(false);  // LCOV_EXCL_LINE
  return true;
}

auto evaluate_expr_impl(const Expr & expr, std::shared_ptr<Environment> env)
  -> std::variant<Value, RuntimeError>
{
  auto evaluator = EvaluateExprVisitor(env);
  return boost::apply_visitor(evaluator, expr);
}

std::optional<RuntimeError> ExecuteStmtVisitor::operator()(const ExprStmt & stmt)
{
  const auto eval_opt = impl::evaluate_expr_impl(stmt.expression, env);
  if (is_variant_v<RuntimeError>(eval_opt)) {
    return as_variant<RuntimeError>(eval_opt);
  }
  return std::nullopt;
}

std::optional<RuntimeError> ExecuteStmtVisitor::operator()(const PrintStmt & stmt)
{
  const auto eval_opt = impl::evaluate_expr_impl(stmt.expression, env);
  if (is_variant_v<RuntimeError>(eval_opt)) {
    return as_variant<RuntimeError>(eval_opt);
  }
  // LCOV_EXCL_START
  std::cout << stringify(as_variant<Value>(eval_opt)) << std::endl;
  // LCOV_EXCL_STOP
  return std::nullopt;
}

std::optional<RuntimeError> ExecuteStmtVisitor::operator()(const Block & block)
{
  /**
   * how it works:
   * when a Block-statement is found, a inner env is created, and it is passed to
   * ExecuteDeclarationVisitor, which may process a Stmt-declaration, and the Stmt-declaration maybe
   * a Block-statement, thus a inner-inner env is created.
   */
  auto sub_scope_env = std::make_shared<Environment>(env);
  for (const auto & declaration : block.declarations) {
    const auto eval_opt =
      boost::apply_visitor(ExecuteDeclarationVisitor(sub_scope_env), declaration);
    if (eval_opt) {
      return eval_opt;
    }
  }
  return std::nullopt;
}

std::optional<RuntimeError> ExecuteStmtVisitor::operator()(const WhileStmt & while_stmt)
{
  while (true) {
    const auto eval_cond_opt = impl::evaluate_expr_impl(while_stmt.cond, env);
    if (is_variant_v<RuntimeError>(eval_cond_opt)) {
      return as_variant<RuntimeError>(eval_cond_opt);
    }
    const auto & cond = as_variant<Value>(eval_cond_opt);
    if (!is_truthy(cond)) {
      return std::nullopt;
    }
    for (const auto & declaration : while_stmt.body) {
      const auto exec_opt = boost::apply_visitor(ExecuteDeclarationVisitor(env), declaration);
      if (exec_opt) {
        return exec_opt;
      }
    }
  }
}

std::variant<bool, RuntimeError> ExecuteStmtVisitor::execute_branch_clause(
  const BranchClause & clause, std::shared_ptr<Environment> if_scope_env)
{
  if (clause.declaration) {
    const auto var_decl_opt = boost::apply_visitor(
      ExecuteDeclarationVisitor(if_scope_env), Declaration{clause.declaration.value()});
    if (var_decl_opt) {
      return var_decl_opt.value();
    }
  }
  const auto cond_opt = impl::evaluate_expr_impl(clause.cond, if_scope_env);
  if (is_variant_v<RuntimeError>(cond_opt)) {
    return as_variant<RuntimeError>(cond_opt);
  }
  const auto & cond = as_variant<Value>(cond_opt);
  if (is_truthy(cond)) {
    for (const auto & declaration : clause.body) {
      const auto exec_opt =
        boost::apply_visitor(ExecuteDeclarationVisitor(if_scope_env), declaration);
      if (exec_opt) {
        return exec_opt.value();
      }
    }
    return true;
  } else {
    return false;
  }
}

std::optional<RuntimeError> ExecuteStmtVisitor::operator()(const IfBlock & if_block)
{
  /* first, top-level if scope environment is created
     this scope is local variable in this function and will be "forgotten"
   */
  auto top_if_scope_env = std::make_shared<Environment>(env);

  const auto execute_if_opt = execute_branch_clause(if_block.if_clause, top_if_scope_env);
  if (is_variant_v<RuntimeError>(execute_if_opt)) {
    return as_variant<RuntimeError>(execute_if_opt);
  }
  const auto if_was_true = as_variant<bool>(execute_if_opt);
  if (if_was_true) {
    return std::nullopt;
  }
  // execute either of the elseif
  std::vector<std::shared_ptr<Environment>> envs{top_if_scope_env};
  for (const auto & elseif_clause : if_block.elseif_clauses) {
    envs.push_back(std::make_shared<Environment>(envs.back()));
    const auto execute_elseif_opt = execute_branch_clause(elseif_clause, envs.back());
    if (is_variant_v<RuntimeError>(execute_elseif_opt)) {
      return as_variant<RuntimeError>(execute_elseif_opt);
    }
    const auto elseif_was_true = as_variant<bool>(execute_elseif_opt);
    if (elseif_was_true) {
      // hit
      return std::nullopt;
    }
  }
  if (if_block.else_body) {
    auto else_scope_env = std::make_shared<Environment>(envs.back());
    // execute the last else
    for (const auto & declaration : if_block.else_body.value()) {
      const auto exec_else_opt =
        boost::apply_visitor(ExecuteDeclarationVisitor(else_scope_env), declaration);
      if (exec_else_opt) {
        return exec_else_opt;
      }
    }
    return std::nullopt;
  }
  return std::nullopt;
}

std::optional<RuntimeError> ExecuteStmtVisitor::operator()(const ForStmt & for_stmt)
{
  // initialization
  auto sub_for_env = std::make_shared<Environment>(env);
  if (for_stmt.init_stmt) {
    const auto & init_stmt = for_stmt.init_stmt.value();
    if (is_variant_v<VarDecl>(init_stmt)) {
      const auto & init_var_stmt = as_variant<VarDecl>(init_stmt);
      impl::ExecuteDeclarationVisitor executor(sub_for_env);
      const auto exec = boost::apply_visitor(executor, Declaration{init_var_stmt});
      if (exec) {
        return exec;
      }
    } else {
      const auto & init_var_stmt = as_variant<ExprStmt>(init_stmt);
      const auto exec = impl::execute_stmt_impl(init_var_stmt, sub_for_env);
      if (exec) {
        return exec;
      }
    }
  }

  auto is_continue_true = [&]() -> std::variant<bool, RuntimeError> {
    if (!for_stmt.cond) {
      return true;
    }
    const auto cond_opt = impl::evaluate_expr_impl(for_stmt.cond.value(), sub_for_env);
    if (is_variant_v<RuntimeError>(cond_opt)) {
      return as_variant<RuntimeError>(cond_opt);
    }
    const auto is_true = is_truthy(as_variant<Value>(cond_opt));
    return is_true;
  };

  auto iterate = [&]() -> std::optional<RuntimeError> {
    if (!for_stmt.next) {
      return std::nullopt;
    }
    const auto exec = impl::evaluate_expr_impl(for_stmt.next.value(), sub_for_env);
    if (is_variant_v<RuntimeError>(exec)) {
      return as_variant<RuntimeError>(exec);
    }
    return std::nullopt;
  };

  while (true) {
    const auto continue_opt = is_continue_true();
    if (is_variant_v<RuntimeError>(continue_opt)) {
      return as_variant<RuntimeError>(continue_opt);
    }
    if (!as_variant<bool>(continue_opt)) {
      break;
    }
    // do the body
    for (const auto & declaration : for_stmt.declarations) {
      const auto exec_opt =
        boost::apply_visitor(ExecuteDeclarationVisitor(sub_for_env), declaration);
      if (exec_opt) {
        return exec_opt;
      }
    }

    const auto iterate_opt = iterate();
    if (iterate_opt) {
      return iterate_opt;
    }
  }

  return std::nullopt;
}

auto execute_stmt_impl(const Stmt & stmt, std::shared_ptr<Environment> env)
  -> std::optional<RuntimeError>
{
  impl::ExecuteStmtVisitor executor(env);
  return boost::apply_visitor(executor, stmt);
}

std::optional<RuntimeError> ExecuteDeclarationVisitor::operator()(const VarDecl & decl)
{
  if (decl.initializer) {
    const auto eval_opt = impl::evaluate_expr_impl(decl.initializer.value(), env);
    if (is_variant_v<RuntimeError>(eval_opt)) {
      return as_variant<RuntimeError>(eval_opt);
    }
    env->define(decl.name, as_variant<Value>(eval_opt));
  } else {
    env->define(decl.name, Nil{});
  }
  return std::nullopt;
}

std::optional<RuntimeError> ExecuteDeclarationVisitor::operator()(const Stmt & stmt)
{
  return execute_stmt_impl(stmt, env);
}  // LCOV_EXCL_LINE

}  // namespace impl

}  // namespace interpreter
}  // namespace lox
