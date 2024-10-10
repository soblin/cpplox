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
  if (match(TokenType::Var)) {
    advance();  // just consume 'var'
    const auto var_declaration = var_decl();
    if (is_variant_v<SyntaxError>(var_declaration)) {
      return as_variant<SyntaxError>(var_declaration);
    }
    return as_variant<VarDecl>(var_declaration);
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

  // <expr_stmt>
  const auto expr_stmt_opt = expr_statement();
  if (is_variant_v<SyntaxError>(expr_stmt_opt)) {
    return as_variant<SyntaxError>(expr_stmt_opt);
  }
  return as_variant<ExprStmt>(expr_stmt_opt);
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
  std::optional<std::vector<Declaration>> else_body;
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
    else_body.emplace(as_variant<Block>(else_body_opt).declarations);
    break;
  }
  return IfBlock{if_clause, elseif_clauses, else_body};
}

auto Parser::branch_clause(const size_t if_start_ctx) -> std::variant<BranchClause, SyntaxError>
{
  if (!match(TokenType::LeftParen)) {
    return create_error(SyntaxErrorKind::MissingIfConditon, current_);
  }
  advance();  // consume '('
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
  const auto & declarations = as_variant<Block>(block_opt).declarations;
  return BranchClause{cond, declarations};
}

auto Parser::expression() -> std::variant<Expr, SyntaxError>
{
  return assignment();
}

auto Parser::assignment() -> std::variant<Expr, SyntaxError>
{
  const auto error_ctx_assign_target = current_;
  const auto left_expr_opt = equality();
  if (is_variant_v<SyntaxError>(left_expr_opt)) {
    return as_variant<SyntaxError>(left_expr_opt);
  }
  if (match(TokenType::Equal)) {
    const auto & lvalue_expr = as_variant<Expr>(left_expr_opt);
    if (!is_variant_v<Variable>(lvalue_expr)) {
      return create_error(SyntaxErrorKind::InvalidAssignmentTarget, error_ctx_assign_target);
    }
    const auto & lvalue = as_variant<Variable>(lvalue_expr);
    advance();  // consume '='
    const auto rvalue_expr = assignment();
    if (is_variant_v<SyntaxError>(rvalue_expr)) {
      return as_variant<SyntaxError>(rvalue_expr);
    }
    const auto & rvalue = as_variant<Expr>(rvalue_expr);
    return Assign{lvalue.name, rvalue};
  }
  return left_expr_opt;
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
  while (match(
    TokenType::Greater, TokenType::GreaterEqual, TokenType::Less, TokenType::LessEqual,
    TokenType::And, TokenType::Or)) {
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
  return primary();
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
