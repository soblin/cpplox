#include <cpplox/parser.hpp>
#include <cpplox/variant.hpp>

#include <boost/variant.hpp>

#include <cassert>

namespace lox
{

inline namespace parser
{

Parser::Parser(const Tokens & tokens) : tokens_(tokens)
{
  if (tokens_.empty() or tokens_.back().type != TokenType::Eof) {
    tokens_.emplace_back(TokenType::Eof, "<EOF>", nullptr, 0);
  }
  assert(tokens_.size() >= 1);
}

auto Parser::program() -> std::variant<Program, SyntaxError>
{
  Program statements;
  while (!is_at_end()) {
    const auto declaration_opt = declaration();
    if (is_variant_v<SyntaxError>(declaration_opt)) {
      return as_variant<SyntaxError>(declaration_opt);
    }
    statements.push_back(as_variant<Declaration>(declaration_opt));
  }
  return statements;
}

auto Parser::declaration() -> std::variant<Declaration, SyntaxError>
{
  // <var_decl>
  if (match(TokenType::Var)) {
    const auto var_declaration = var_decl();
    if (is_variant_v<SyntaxError>(var_declaration)) {
      return as_variant<SyntaxError>(var_declaration);
    }
    return as_variant<VarDecl>(var_declaration);
  }

  // <func_decl>
  if (match(TokenType::Fun)) {
    const auto func_decl_opt = func_decl();
    if (is_variant_v<SyntaxError>(func_decl_opt)) {
      return as_variant<SyntaxError>(func_decl_opt);
    }
    return as_variant<FuncDecl>(func_decl_opt);
  }

  // <class_decl>
  if (match(TokenType::Class)) {
    const auto class_decl_opt = class_decl();
    if (is_variant_v<SyntaxError>(class_decl_opt)) {
      return as_variant<SyntaxError>(class_decl_opt);
    }
    return as_variant<ClassDecl>(class_decl_opt);
  }

  const auto statement_opt = statement();
  if (is_variant_v<SyntaxError>(statement_opt)) {
    return as_variant<SyntaxError>(statement_opt);
  }
  const auto & stmt = as_variant<Stmt>(statement_opt);
  Declaration decl = stmt;
  return decl;
}

auto Parser::var_decl() -> std::variant<VarDecl, SyntaxError>
{
  advance();  // just consume 'var'
  const auto var_decl_ctx = current_;
  if (!match(TokenType::Identifier)) {
    return create_error(SyntaxErrorKind::MissingValidIdentifierDecl, var_decl_ctx);
  }
  const auto & name = peek();  // save identifier
  advance();                   // consume IDENTIFIER
  if (match(TokenType::Equal)) {
    advance();  // consume '='
    const auto right_expr_opt = expression();
    if (is_variant_v<SyntaxError>(right_expr_opt)) {
      return as_variant<SyntaxError>(right_expr_opt);
    }
    const auto initializer = as_variant<Expr>(right_expr_opt);
    if (!match(TokenType::Semicolun)) {
      return create_error(SyntaxErrorKind::StmtWithoutSemicolun, var_decl_ctx);
    }
    advance();  // consume ';'
    return VarDecl{name, std::make_optional<Expr>(initializer)};
  }
  if (!match(TokenType::Semicolun)) {
    return create_error(SyntaxErrorKind::StmtWithoutSemicolun, var_decl_ctx);
  }
  advance();  // consume ';'
  return VarDecl{name, std::nullopt};
}

auto Parser::statement() -> std::variant<Stmt, SyntaxError>
{
  // <print_stmt>
  if (match(TokenType::Print)) {
    advance();  // consumme 'print'
    const auto print_stmt_opt = print_statement();
    if (is_variant_v<SyntaxError>(print_stmt_opt)) {
      return as_variant<SyntaxError>(print_stmt_opt);
    }
    return as_variant<PrintStmt>(print_stmt_opt);
  }

  // <block>
  if (match(TokenType::LeftBrace)) {
    const auto block_opt = block();
    if (is_variant_v<SyntaxError>(block_opt)) {
      return as_variant<SyntaxError>(block_opt);
    }
    return as_variant<Block>(block_opt);
  }

  // <if_block>
  const auto if_start_ctx = current_;
  if (match(TokenType::If)) {
    advance();  // consume "if"
    const auto if_block_opt = if_block(if_start_ctx);
    if (is_variant_v<SyntaxError>(if_block_opt)) {
      return as_variant<SyntaxError>(if_block_opt);
    }
    return as_variant<IfBlock>(if_block_opt);
  }

  // <while_stmt>
  if (match(TokenType::While)) {
    const auto while_start_ctx = current_;
    advance();  // consume "while"
    const auto while_block_opt = while_stmt(while_start_ctx);
    if (is_variant_v<SyntaxError>(while_block_opt)) {
      return as_variant<SyntaxError>(while_block_opt);
    }
    return as_variant<WhileStmt>(while_block_opt);
  }

  // <for_stmt>
  if (match(TokenType::For)) {
    const auto for_start_ctx = current_;
    advance();  // consume "for"
    const auto for_stmt_opt = for_stmt(for_start_ctx);
    if (is_variant_v<SyntaxError>(for_stmt_opt)) {
      return as_variant<SyntaxError>(for_stmt_opt);
    }
    return as_variant<ForStmt>(for_stmt_opt);
  }

  // <break_stmt>
  if (match(TokenType::Break)) {
    const auto stmt_opt = break_stmt();
    if (is_variant_v<SyntaxError>(stmt_opt)) {
      return as_variant<SyntaxError>(stmt_opt);
    }
    return as_variant<BreakStmt>(stmt_opt);
  }

  // <continue_stmt>
  if (match(TokenType::Continue)) {
    const auto stmt_opt = continue_stmt();
    if (is_variant_v<SyntaxError>(stmt_opt)) {
      return as_variant<SyntaxError>(stmt_opt);
    }
    return as_variant<ContinueStmt>(stmt_opt);
  }

  // <return_stmt>
  if (match(TokenType::Return)) {
    const auto stmt_opt = return_stmt();
    if (is_variant_v<SyntaxError>(stmt_opt)) {
      return as_variant<SyntaxError>(stmt_opt);
    }
    return as_variant<ReturnStmt>(stmt_opt);
  }

  // <expr_stmt>
  const auto expr_stmt_opt = expr_statement();
  if (is_variant_v<SyntaxError>(expr_stmt_opt)) {
    return as_variant<SyntaxError>(expr_stmt_opt);
  }
  return as_variant<ExprStmt>(expr_stmt_opt);
}

auto Parser::func_decl() -> std::variant<FuncDecl, SyntaxError>
{
  advance();                 // consume "fun"
  const auto name = peek();  // function name after "fun"
  advance();                 // consume function name
  const auto fun_ctx = current_;
  if (!match(TokenType::LeftParen)) {
    return create_error(SyntaxErrorKind::MissingFuncParameterDecl, fun_ctx);
  }
  advance();  // consume '('
  Tokens parameters{};
  while (true) {
    if (match(TokenType::RightParen)) {
      break;
    }
    if (!match(TokenType::Identifier)) {
      return create_error(SyntaxErrorKind::InvalidParameterDecl, current_);
    }
    parameters.push_back(advance());
    if (match(TokenType::Comma)) {
      advance();  // consume ','
    } else if (match(TokenType::RightParen)) {
      break;
    }
    if (parameters.size() >= max_argument_size) {
      return create_error(SyntaxErrorKind::TooManyArguments, fun_ctx);
    }
  }
  advance();  // consume ')'
  if (!match(TokenType::LeftBrace)) {
    return create_error(SyntaxErrorKind::MissingFuncBodyDecl, current_);
  }
  const auto block_opt = block();
  if (is_variant_v<SyntaxError>(block_opt)) {
    return as_variant<SyntaxError>(block_opt);
  }
  return FuncDecl{name, parameters, as_variant<Block>(block_opt)};
}

auto Parser::class_decl() -> std::variant<ClassDecl, SyntaxError>
{
  advance();  // consume "class"
  const auto name = peek();
  advance();  // consume class-name
  const auto class_ctx = current_;
  if (!match(TokenType::LeftBrace)) {
    return create_error(SyntaxErrorKind::MissingClassBodyDecl, class_ctx);
  }
  advance();  // consume "{"
  std::unordered_map<std::string_view, FuncDecl> methods;
  while (true) {
    if (match(TokenType::RightBrace)) {
      advance();  // consume "}"
      break;
    }
    const auto method_opt = func_decl();
    if (is_variant_v<SyntaxError>(method_opt)) {
      return as_variant<SyntaxError>(method_opt);
    }
    const auto & method = as_variant<FuncDecl>(method_opt);
    // TODO(soblin): deal with overloads
    methods.emplace(method.name.lexeme, method);
  }
  return ClassDecl{name, methods};
}

auto Parser::expr_statement() -> std::variant<ExprStmt, SyntaxError>
{
  const auto expr_ctx = current_;
  const auto expr_opt = expression();
  if (is_variant_v<SyntaxError>(expr_opt)) {
    return as_variant<SyntaxError>(expr_opt);
  }
  if (match(TokenType::Semicolun)) {
    advance();  // just consime ';'
    return ExprStmt{as_variant<Expr>(expr_opt)};
  }
  return create_error(SyntaxErrorKind::StmtWithoutSemicolun, expr_ctx);
}

auto Parser::print_statement() -> std::variant<PrintStmt, SyntaxError>
{
  const auto print_ctx = current_;
  const auto expr_opt = expression();
  if (match(TokenType::Semicolun)) {
    advance();  // just consume ';'
    return PrintStmt{as_variant<Expr>(expr_opt)};
  } else {
    return create_error(SyntaxErrorKind::StmtWithoutSemicolun, print_ctx);
  }
}

auto Parser::block() -> std::variant<Block, SyntaxError>
{
  const auto brace_ctx = current_;
  advance();  // consume '{'
  std::vector<Declaration> declarations;

  // TODO(soblin): nice way to handle empty Block
  if (match(TokenType::RightBrace)) {
    advance();  // consume '}'
    return Block{declarations};
  }

  while (!is_at_end()) {
    const auto decl_opt = declaration();
    if (is_variant_v<Declaration>(decl_opt)) {
      const auto & decl = as_variant<Declaration>(decl_opt);
      declarations.push_back(decl);
      if (match(TokenType::RightBrace)) {
        break;
      }
    } else {
      return as_variant<SyntaxError>(decl_opt);
    }
  }
  if (!match(TokenType::RightBrace)) {
    return create_error(SyntaxErrorKind::UnmatchedBraceError, brace_ctx);
  }
  advance();  // consume '}'
  return Block{declarations};
}

auto Parser::if_block(const size_t if_start_ctx) -> std::variant<IfBlock, SyntaxError>
{
  const auto branch_clause_opt = branch_clause(if_start_ctx);
  if (is_variant_v<SyntaxError>(branch_clause_opt)) {
    return as_variant<SyntaxError>(branch_clause_opt);
  }
  const auto & if_clause = as_variant<BranchClause>(branch_clause_opt);
  std::vector<BranchClause> elseif_clauses;
  std::optional<Block> else_body;
  while (!is_at_end()) {
    const auto else_start_ctx = current_;
    if (!match(TokenType::Else)) {
      // end
      break;
    }
    advance();  // consume "else"

    // "else if () {}"
    if (match(TokenType::If)) {
      advance();  // consume "if"
      const auto elseif_branch_clause_opt = branch_clause(else_start_ctx);
      if (is_variant_v<SyntaxError>(elseif_branch_clause_opt)) {
        return as_variant<SyntaxError>(elseif_branch_clause_opt);
      }
      elseif_clauses.push_back(as_variant<BranchClause>(elseif_branch_clause_opt));
      continue;
    }

    // "else {}"
    if (!match(TokenType::LeftBrace)) {
      return create_error(SyntaxErrorKind::UnmatchedBraceError, else_start_ctx);
    }
    const auto else_body_opt = block();
    if (is_variant_v<SyntaxError>(else_body_opt)) {
      return as_variant<SyntaxError>(else_body_opt);
    }
    else_body.emplace(as_variant<Block>(else_body_opt));
    break;
  }
  return IfBlock{if_clause, elseif_clauses, else_body};
}

auto Parser::while_stmt(const size_t while_start_ctx) -> std::variant<WhileStmt, SyntaxError>
{
  if (!match(TokenType::LeftParen)) {
    return create_error(SyntaxErrorKind::MissingWhileConditon, current_);
  }
  advance();  // consume '('
  const auto cond_opt = expression();
  if (is_variant_v<SyntaxError>(cond_opt)) {
    return as_variant<SyntaxError>(cond_opt);
  }
  const auto & cond = as_variant<Expr>(cond_opt);
  if (!match(TokenType::RightParen)) {
    return create_error(SyntaxErrorKind::UnmatchedParenError, while_start_ctx);
  }
  advance();  // consume ')'
  if (!match(TokenType::LeftBrace)) {
    return create_error(SyntaxErrorKind::MissingWhileBody, while_start_ctx);
  }
  const auto block_opt = block();
  if (is_variant_v<SyntaxError>(block_opt)) {
    return as_variant<SyntaxError>(block_opt);
  }
  return WhileStmt{tokens_.at(while_start_ctx), cond, as_variant<Block>(block_opt)};
}

auto Parser::for_stmt(const size_t for_start_ctx) -> std::variant<ForStmt, SyntaxError>
{
  if (!match(TokenType::LeftParen)) {
    return create_error(SyntaxErrorKind::MissingForCondition, current_);
  }
  advance();  // consume '('

  std::optional<std::variant<VarDecl, ExprStmt>> init_stmt{std::nullopt};
  if (match(TokenType::Semicolun)) {
    advance();  // consume ';'
    // do nothing
  } else if (match(TokenType::Var)) {
    const auto var_decl_opt = var_decl();
    if (is_variant_v<SyntaxError>(var_decl_opt)) {
      return as_variant<SyntaxError>(var_decl_opt);
    }
    init_stmt.emplace(as_variant<VarDecl>(var_decl_opt));
  } else {
    const auto expr_stmt_opt = expr_statement();
    if (is_variant_v<SyntaxError>(expr_stmt_opt)) {
      return as_variant<SyntaxError>(expr_stmt_opt);
    }
    init_stmt.emplace(as_variant<ExprStmt>(expr_stmt_opt));
  }

  std::optional<Expr> cond{std::nullopt};
  if (match(TokenType::Semicolun)) {
    advance();  // consume ';'
  } else {
    const auto expr_ctx = current_;
    const auto expr_opt = expression();
    if (is_variant_v<SyntaxError>(expr_opt)) {
      return as_variant<SyntaxError>(expr_opt);
    }
    if (!match(TokenType::Semicolun)) {
      return create_error(SyntaxErrorKind::StmtWithoutSemicolun, expr_ctx);
    }
    advance();  // consume ';'
    cond.emplace(as_variant<Expr>(expr_opt));
  }

  std::optional<Expr> next{std::nullopt};
  if (match(TokenType::RightParen)) {
    advance();  // consume ')'
  } else {
    const auto expr_ctx = current_;
    const auto expr_opt = expression();
    if (is_variant_v<SyntaxError>(expr_opt)) {
      return as_variant<SyntaxError>(expr_opt);
    }
    if (!match(TokenType::RightParen)) {
      return create_error(SyntaxErrorKind::UnmatchedParenError, expr_ctx);
    }
    advance();  // consume ')'
    next.emplace(as_variant<Expr>(expr_opt));
  }

  if (!match(TokenType::LeftBrace)) {
    return create_error(SyntaxErrorKind::MissingForBody, current_);
  }
  const auto block_opt = block();
  if (is_variant_v<SyntaxError>(block_opt)) {
    return as_variant<SyntaxError>(block_opt);
  }
  return ForStmt{tokens_.at(for_start_ctx), init_stmt, cond, next, as_variant<Block>(block_opt)};
}

auto Parser::break_stmt() -> std::variant<BreakStmt, SyntaxError>
{
  const size_t ctx = current_;
  advance();  // consume "break"
  if (!match(TokenType::Semicolun)) {
    return create_error(SyntaxErrorKind::StmtWithoutSemicolun, ctx);
  }
  advance();  // consume ';'
  return BreakStmt{};
}

auto Parser::continue_stmt() -> std::variant<ContinueStmt, SyntaxError>
{
  const size_t ctx = current_;
  advance();  // consume "continue"
  if (!match(TokenType::Semicolun)) {
    return create_error(SyntaxErrorKind::StmtWithoutSemicolun, ctx);
  }
  advance();  // consume ';'
  return ContinueStmt{};
}

auto Parser::return_stmt() -> std::variant<ReturnStmt, SyntaxError>
{
  const auto return_ctx = current_;
  advance();  // consume "return"
  if (match(TokenType::Semicolun)) {
    advance();  // consume ";'"
    return ReturnStmt{std::nullopt};
  }
  const auto expr_opt = expression();
  if (is_variant_v<SyntaxError>(expr_opt)) {
    return as_variant<SyntaxError>(expr_opt);
  }
  if (!match(TokenType::Semicolun)) {
    return create_error(SyntaxErrorKind::StmtWithoutSemicolun, return_ctx);
  }
  advance();  // consume ';'
  return ReturnStmt{as_variant<Expr>(expr_opt)};
}

auto Parser::branch_clause(const size_t if_start_ctx) -> std::variant<BranchClause, SyntaxError>
{
  if (!match(TokenType::LeftParen)) {
    return create_error(SyntaxErrorKind::MissingIfConditon, current_);
  }
  advance();  // consume '('
  std::optional<VarDecl> decl = std::nullopt;
  if (match(TokenType::Var)) {
    const auto & var_decl_opt = var_decl();
    if (is_variant_v<SyntaxError>(var_decl_opt)) {
      return as_variant<SyntaxError>(var_decl_opt);
    }
    decl.emplace(as_variant<VarDecl>(var_decl_opt));
  }
  const auto cond_opt = expression();
  if (is_variant_v<SyntaxError>(cond_opt)) {
    return as_variant<SyntaxError>(cond_opt);
  }
  const auto & cond = as_variant<Expr>(cond_opt);
  if (!match(TokenType::RightParen)) {
    return create_error(SyntaxErrorKind::UnmatchedParenError, if_start_ctx);
  }
  advance();  // consume ')'
  if (!match(TokenType::LeftBrace)) {
    return create_error(SyntaxErrorKind::MissingIfBody, if_start_ctx);
  }
  const auto block_opt = block();
  if (is_variant_v<SyntaxError>(block_opt)) {
    return as_variant<SyntaxError>(block_opt);
  }
  return BranchClause{decl, cond, as_variant<Block>(block_opt)};
}

auto Parser::expression() -> std::variant<Expr, SyntaxError>
{
  return assignment();
}

auto Parser::assignment() -> std::variant<Expr, SyntaxError>
{
  const auto error_ctx_assign_target = current_;
  const auto left_expr_opt = logic_or();
  if (is_variant_v<SyntaxError>(left_expr_opt)) {
    return as_variant<SyntaxError>(left_expr_opt);
  }
  if (match(TokenType::Equal)) {
    const auto & lvalue_expr = as_variant<Expr>(left_expr_opt);
    if (!is_variant_v<Variable>(lvalue_expr) && !is_variant_v<ReadProperty>(lvalue_expr)) {
      return create_error(SyntaxErrorKind::InvalidAssignmentTarget, error_ctx_assign_target);
    }
    advance();  // consume '='
    const auto rvalue_expr = assignment();
    if (is_variant_v<SyntaxError>(rvalue_expr)) {
      return as_variant<SyntaxError>(rvalue_expr);
    }
    const auto & rvalue = as_variant<Expr>(rvalue_expr);
    if (is_variant_v<Variable>(lvalue_expr)) {
      const auto & lvalue = as_variant<Variable>(lvalue_expr);
      return Assign{lvalue.name, rvalue};
    } else {
      const auto & lvalue = as_variant<ReadProperty>(lvalue_expr);
      return SetProperty{lvalue.base, lvalue.prop, rvalue};
    }
  }
  return left_expr_opt;
}

auto Parser::logic_or() -> std::variant<Expr, SyntaxError>
{
  const auto logic_and_opt = logic_and();
  if (is_variant_v<SyntaxError>(logic_and_opt)) {
    return as_variant<SyntaxError>(logic_and_opt);
  }
  const auto & left = as_variant<Expr>(logic_and_opt);
  std::vector<Expr> exprs{left};
  while (match(TokenType::Or)) {
    const auto & op = advance();
    const auto next_logic_and_opt = logic_and();
    if (is_variant_v<SyntaxError>(next_logic_and_opt)) {
      return as_variant<SyntaxError>(next_logic_and_opt);
    }
    const auto logical = Logical{exprs.back(), op, as_variant<Expr>(next_logic_and_opt)};
    exprs.push_back(logical);
  }
  return exprs.back();
}

auto Parser::logic_and() -> std::variant<Expr, SyntaxError>
{
  const auto eq_opt = equality();
  if (is_variant_v<SyntaxError>(eq_opt)) {
    return as_variant<SyntaxError>(eq_opt);
  }
  const auto & eq = as_variant<Expr>(eq_opt);
  std::vector<Expr> exprs{eq};
  while (match(TokenType::And)) {
    const auto & op = advance();
    const auto next_eq_opt = equality();
    if (is_variant_v<SyntaxError>(next_eq_opt)) {
      return as_variant<SyntaxError>(next_eq_opt);
    }
    const auto logical = Logical{exprs.back(), op, as_variant<Expr>(next_eq_opt)};
    exprs.push_back(logical);
  }
  return exprs.back();
}

auto Parser::equality() -> std::variant<Expr, SyntaxError>
{
  const auto expr_opt = comparison();
  if (is_variant_v<SyntaxError>(expr_opt)) {
    return expr_opt;
  }
  std::vector<Expr> exprs{as_variant<Expr>(expr_opt)};
  while (match(TokenType::BangEqual, TokenType::EqualEqual)) {
    const auto & op = advance();
    const auto right_opt = comparison();
    if (is_variant_v<SyntaxError>(right_opt)) {
      return right_opt;
    }
    const auto binary = Binary{exprs.back(), op, as_variant<Expr>(right_opt)};
    exprs.push_back(binary);
  }
  return exprs.back();
}

auto Parser::comparison() -> std::variant<Expr, SyntaxError>
{
  const auto expr_opt = term();
  if (is_variant_v<SyntaxError>(expr_opt)) {
    return expr_opt;
  }
  std::vector<Expr> exprs{as_variant<Expr>(expr_opt)};
  while (
    match(TokenType::Greater, TokenType::GreaterEqual, TokenType::Less, TokenType::LessEqual)) {
    const auto & op = advance();
    const auto right_opt = term();
    if (is_variant_v<SyntaxError>(right_opt)) {
      return right_opt;
    }
    const auto binary = Binary{exprs.back(), op, as_variant<Expr>(right_opt)};
    exprs.push_back(binary);
  }
  return exprs.back();
}

auto Parser::term() -> std::variant<Expr, SyntaxError>
{
  const auto factor_opt = factor();
  if (is_variant_v<SyntaxError>(factor_opt)) {
    return factor_opt;
  }
  std::vector<Expr> exprs{as_variant<Expr>(factor_opt)};
  while (match(TokenType::Plus, TokenType::Minus)) {
    const auto & op = advance();
    const auto right_opt = factor();
    if (is_variant_v<SyntaxError>(right_opt)) {
      return right_opt;
    }
    const auto binary = Binary{exprs.back(), op, as_variant<Expr>(right_opt)};
    exprs.push_back(binary);
  }
  return exprs.back();
}

auto Parser::factor() -> std::variant<Expr, SyntaxError>
{
  const auto unary_opt = unary();
  if (is_variant_v<SyntaxError>(unary_opt)) {
    return unary_opt;
  }
  std::vector<Expr> exprs{as_variant<Expr>(unary_opt)};
  while (match(TokenType::Slash, TokenType::Star)) {
    const auto & op = advance();
    const auto right_opt = unary();
    if (is_variant_v<SyntaxError>(right_opt)) {
      return right_opt;
    }
    const auto binary = Binary{exprs.back(), op, as_variant<Expr>(right_opt)};
    exprs.push_back(binary);
  }
  return exprs.back();
}

auto Parser::unary() -> std::variant<Expr, SyntaxError>
{
  if (match(TokenType::Bang, TokenType::Minus)) {
    const auto & op = advance();
    const auto unary_next_opt = unary();
    if (is_variant_v<SyntaxError>(unary_next_opt)) {
      return unary_next_opt;
    }
    return Unary{op, as_variant<Expr>(unary_next_opt)};
  }
  return call();
}

auto Parser::call() -> std::variant<Expr, SyntaxError>
{
  const auto primary_opt = primary();
  if (is_variant_v<SyntaxError>(primary_opt)) {
    return as_variant<SyntaxError>(primary_opt);
  }
  const auto & prim = as_variant<Expr>(primary_opt);
  std::vector<Expr> exprs{prim};
  while (true) {
    if (match(TokenType::LeftParen)) {
      const auto paren_ctx = current_;
      advance();  // consume '('
      const auto arguments_opt = arguments();
      if (is_variant_v<SyntaxError>(arguments_opt)) {
        return as_variant<SyntaxError>(arguments_opt);
      }
      const auto caller = Call{exprs.back(), as_variant<std::vector<Expr>>(arguments_opt)};
      if (!match(TokenType::RightParen)) {
        create_error(SyntaxErrorKind::UnmatchedParenError, paren_ctx);
      }
      exprs.push_back(caller);
      advance();  // consume ')'
    } else if (match(TokenType::Dot)) {
      advance();  // consume '.'
      const auto r_prop = ReadProperty{exprs.back(), peek()};
      advance();  // consume property-name after '.'
      exprs.push_back(r_prop);
    } else {
      break;
    }
  }
  return exprs.back();
}

auto Parser::arguments() -> std::variant<std::vector<Expr>, SyntaxError>
{
  std::vector<Expr> args;
  const auto args_ctx = current_;
  if (!match(TokenType::RightParen)) {
    while (true) {
      const auto arg_opt = expression();
      if (is_variant_v<SyntaxError>(arg_opt)) {
        return as_variant<SyntaxError>(arg_opt);
      }
      args.push_back(as_variant<Expr>(arg_opt));
      if (!match(TokenType::Comma)) {
        break;
      } else {
        advance();  // consume ','
      }
      if (args.size() >= max_argument_size) {
        return create_error(SyntaxErrorKind::TooManyArguments, args_ctx);
      }
    }
  }
  return args;
}

auto Parser::primary() -> std::variant<Expr, SyntaxError>
{
  const auto error_ctx_primary = current_;
  if (match(
        TokenType::Number, TokenType::String, TokenType::True, TokenType::False, TokenType::Nil)) {
    const auto & token = advance();
    return Literal{token.type, token.lexeme, token.line, token.start_index};
  }
  if (match(TokenType::Identifier)) {
    const auto & token = advance();
    return Variable{token};
  }
  if (match(TokenType::LeftParen)) {
    const auto left_paren = peek();
    const auto error_ctx_paren = current_;
    const auto left_anchor = peek();
    advance();  // just consume '('
    const auto expr_opt = expression();
    if (is_variant_v<SyntaxError>(expr_opt)) {
      return expr_opt;
    }
    if (match(TokenType::RightParen)) {
      const auto right_paren = peek();
      advance();  // just consume ')'
      return Group{left_paren, as_variant<Expr>(expr_opt), right_paren};
    }
    return create_error(SyntaxErrorKind::UnmatchedParenError, error_ctx_paren);
  }
  return create_error(SyntaxErrorKind::InvalidLiteralError, error_ctx_primary);
}

auto Parser::is_at_end() const noexcept -> bool
{
  /**
     NOTE: not check by `current_ == tokens_.size()`, because otherwise 'EOF' is evaluated as
     primary() which is troublesome
  */
  if (current_ == (tokens_.size() - 1)) {
    assert(peek().type == TokenType::Eof);
    return true;
  }
  return false;
}

auto Parser::peek() const -> const Token &
{
  return tokens_.at(current_);
}

auto Parser::advance() -> const Token &
{
  if (!is_at_end()) {
    current_++;
  }
  return tokens_.at(current_ - 1);
}

auto Parser::create_error(const SyntaxErrorKind & kind, const size_t error_ctx) const -> SyntaxError
{
  const auto ctx_token = tokens_.at(error_ctx);
  return SyntaxError{
    kind, ctx_token.line, ctx_token.start_index, ctx_token.start_index + ctx_token.lexeme.size()};
}

}  // namespace parser
}  // namespace lox
