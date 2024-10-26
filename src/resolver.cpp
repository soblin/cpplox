#include <cpplox/resolver.hpp>

namespace lox
{
inline namespace resolver
{

std::optional<CompileError> StmtResolver::operator()(const ExprStmt & stmt)
{
  ExprResolver resolver(scopes, lookup);
  return boost::apply_visitor(resolver, stmt.expression);
}

std::optional<CompileError> StmtResolver::operator()(const PrintStmt & stmt)
{
  ExprResolver resolver(scopes, lookup);
  return boost::apply_visitor(resolver, stmt.expression);
}

std::optional<CompileError> StmtResolver::operator()(const Block & block)
{
  begin_scope();
  DeclResolver decl_resolver(scopes, lookup);
  for (const auto & declaration : block.declarations) {
    if (const auto err = boost::apply_visitor(decl_resolver, declaration); err) {
      return err;
    }
  }
  end_scope();
  return std::nullopt;
}

std::optional<CompileError> StmtResolver::operator()(const IfBlock & stmt)
{
  size_t n_nest_call = 0;
  auto resolve_branch_clause = [&](const BranchClause & clause) -> std::optional<CompileError> {
    begin_scope();
    n_nest_call++;
    if (clause.declaration) {
      DeclResolver resolver(scopes, lookup);
      if (const auto err = boost::apply_visitor(resolver, Declaration{clause.declaration.value()});
          err) {
        return err;
      }
    }
    ExprResolver resolver(scopes, lookup);
    if (const auto err = boost::apply_visitor(resolver, clause.cond); err) {
      return err;
    }
    return boost::apply_visitor(*this, Stmt{clause.body});
  };

  // for if
  if (const auto err = resolve_branch_clause(stmt.if_clause); err) {
    return err;
  }
  for (const auto & elseif_clause : stmt.elseif_clauses) {
    if (const auto err = resolve_branch_clause(elseif_clause); err) {
      return err;
    }
  }
  if (stmt.else_body) {
    n_nest_call++;
    begin_scope();
    if (const auto err = boost::apply_visitor(*this, Stmt{stmt.else_body.value()}); err) {
      return err;
    }
  }
  for (unsigned i = 1; i <= n_nest_call; ++i) {
    end_scope();
  }
  return std::nullopt;
}

std::optional<CompileError> StmtResolver::operator()(const WhileStmt & stmt)
{
  ExprResolver expr_resolver(scopes, lookup);
  if (const auto err = boost::apply_visitor(expr_resolver, stmt.cond); err) {
    return err;
  }
  begin_scope();
  StmtResolver stmt_resolver(scopes, lookup);
  if (const auto err = boost::apply_visitor(stmt_resolver, Stmt{stmt.body}); err) {
    return err;
  }
  end_scope();
  return std::nullopt;
}

std::optional<CompileError> StmtResolver::operator()(const ForStmt & stmt)
{
  begin_scope();
  if (stmt.init_stmt) {
    const auto & init_stmt = stmt.init_stmt.value();
    if (is_variant_v<VarDecl>(init_stmt)) {
      const auto & var_stmt = as_variant<VarDecl>(init_stmt);
      DeclResolver resolver(scopes, lookup);
      if (const auto err = boost::apply_visitor(resolver, Declaration{var_stmt}); err) {
        return err;
      }
    } else if (is_variant_v<ExprStmt>(init_stmt)) {
      const auto & expr_stmt = as_variant<ExprStmt>(init_stmt);
      ExprResolver expr_resolver(scopes, lookup);
      if (const auto err = boost::apply_visitor(*this, Stmt{expr_stmt}); err) {
        return err;
      }
    }
  }
  if (stmt.cond) {
    ExprResolver resolver(scopes, lookup);
    if (const auto err = boost::apply_visitor(resolver, stmt.cond.value()); err) {
      return err;
    }
  }
  if (stmt.next) {
    ExprResolver resolver(scopes, lookup);
    if (const auto err = boost::apply_visitor(resolver, stmt.next.value()); err) {
      return err;
    }
  }
  if (const auto err = boost::apply_visitor(*this, Stmt{stmt.body}); err) {
    return err;
  }
  end_scope();

  return std::nullopt;
}

std::optional<CompileError> StmtResolver::operator()(const BreakStmt & stmt)
{
  return std::nullopt;
}

std::optional<CompileError> StmtResolver::operator()(const ContinueStmt & stmt)
{
  return std::nullopt;
}

std::optional<CompileError> StmtResolver::operator()(const ReturnStmt & stmt)
{
  if (stmt.expr) {
    ExprResolver resolver(scopes, lookup);
    if (const auto err = boost::apply_visitor(resolver, stmt.expr.value()); err) {
      return err;
    }
  }
  return std::nullopt;
}

void StmtResolver::begin_scope()
{
  scopes.push_back({});
}

void StmtResolver::end_scope()
{
  scopes.pop_back();
}

std::optional<CompileError> DeclResolver::operator()(const VarDecl & var_decl)
{
  declare(var_decl.name);
  if (var_decl.initializer) {
    ExprResolver expr_resolver(scopes, lookup);
    if (const auto err = boost::apply_visitor(expr_resolver, var_decl.initializer.value()); err) {
      return err;
    }
  }
  define(var_decl.name);
  return std::nullopt;
}

std::optional<CompileError> DeclResolver::operator()(const Stmt & stmt)
{
  StmtResolver resolver(scopes, lookup);
  return boost::apply_visitor(resolver, stmt);
}

std::optional<CompileError> DeclResolver::operator()(const FuncDecl & func_decl)
{
  declare(func_decl.name);
  define(func_decl.name);

  begin_scope();
  for (const auto & param : func_decl.parameters) {
    declare(param);
    define(param);
  }
  StmtResolver body_resolver(scopes, lookup);
  if (const auto err = boost::apply_visitor(body_resolver, Stmt{func_decl.body}); err) {
    return err;
  }
  end_scope();
  return std::nullopt;
}

void DeclResolver::begin_scope()
{
  scopes.push_back({});
}

void DeclResolver::end_scope()
{
  scopes.pop_back();
}

void DeclResolver::declare(const Token & name)
{
  if (scopes.empty()) {
    return;
  }
  scopes.back()[name.lexeme] = false;
}

void DeclResolver::define(const Token & name)
{
  if (scopes.empty()) {
    return;
  }
  scopes.back()[name.lexeme] = true;
}

std::optional<CompileError> ExprResolver::operator()(const Literal & literal)
{
  return std::nullopt;
}

std::optional<CompileError> ExprResolver::operator()(const Unary & unary)
{
  return boost::apply_visitor(*this, unary.expr);
}

std::optional<CompileError> ExprResolver::operator()(const Binary & binary)
{
  if (const auto err = boost::apply_visitor(*this, binary.left); err) {
    return err;
  }
  return boost::apply_visitor(*this, binary.right);
}

std::optional<CompileError> ExprResolver::operator()(const Group & group)
{
  return boost::apply_visitor(*this, group.expr);
}

std::optional<CompileError> ExprResolver::operator()(const Variable & expr)
{
  if (!scopes.empty()) {
    const auto it = scopes.back().find(expr.name.lexeme);
    if (it != scopes.back().end() && it->second == false) {
      return UndefVariableError{expr.name};
    }
  }

  if (scopes.empty()) {
    return std::nullopt;
  }

  resolve_local(expr.name);
  return std::nullopt;
}

std::optional<CompileError> ExprResolver::operator()(const Assign & assign)
{
  if (const auto err = boost::apply_visitor(*this, assign.expr); err) {
    return err;
  }
  resolve_local(assign.name);
  return std::nullopt;
}

std::optional<CompileError> ExprResolver::operator()(const Logical & logical)
{
  if (const auto err = boost::apply_visitor(*this, logical.left); err) {
    return err;
  }
  return boost::apply_visitor(*this, logical.right);
}

std::optional<CompileError> ExprResolver::operator()(const Call & call)
{
  if (const auto err = boost::apply_visitor(*this, call.callee); err) {
    return err;
  }
  for (const auto & argument : call.arguments) {
    if (const auto err = boost::apply_visitor(*this, argument); err) {
      return err;
    }
  }
  return std::nullopt;
}

void ExprResolver::resolve_local(const Token & name)
{
  for (int i = scopes.size() - 1; i >= 0; i--) {
    if (const auto it = scopes.at(i).find(name.lexeme); it != scopes.at(i).end()) {
      // if depth == 0, do not traverse enclosing
      lookup[name] = scopes.size() - 1 - i;
      return;
    }
  }
}

}  // namespace resolver
}  // namespace lox
