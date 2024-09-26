#include "cpplox/statement.hpp"
#include <cpplox/error.hpp>
#include <cpplox/parser.hpp>
#include <cpplox/variant.hpp>

#include <cassert>

namespace lox
{

inline namespace parser
{

Parser::Parser(const Tokens & tokens) : tokens_(tokens)
{
  if (tokens_.empty() or tokens_.back().type != TokenType::Eof) {
    tokens_.emplace_back(TokenType::Eof, "<EOF>", 0, 0);
  }
  assert(tokens_.size() >= 1);
}

auto Parser::program() -> std::variant<std::vector<Stmt>, SyntaxError>
{
  std::vector<Stmt> statements;
  while (!is_at_end()) {
    const auto statement_opt = declaration();
    if (is_variant_v<SyntaxError>(statement_opt)) {
      return as_variant<SyntaxError>(statement_opt);
    }
    statements.push_back(as_variant<Stmt>(statement_opt));
  }
  return statements;
}

auto Parser::declaration() -> std::variant<Stmt, SyntaxError>
{
  if (match(TokenType::Var)) {
    advance();  // just consume 'var'
    return var_decl();
  }
  return statement();
}

auto Parser::var_decl() -> std::variant<Stmt, SyntaxError>
{
  if (!match(TokenType::Identifier)) {
    return SyntaxError{SyntaxErrorKind::MissingValidIdentifierDecl, peek().line, peek().column};
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
      SyntaxError{SyntaxErrorKind::StmtWithoutSemicolun, peek().line, peek().column};
    }
    advance();  // consume ';'
    return VarDeclStmt{name, std::make_optional<Expr>(initializer)};
  }
  if (!match(TokenType::Semicolun)) {
    SyntaxError{SyntaxErrorKind::StmtWithoutSemicolun, peek().line, peek().column};
  }
  advance();  // consume ';'
  return VarDeclStmt{name, std::nullopt};
}

auto Parser::statement() -> std::variant<Stmt, SyntaxError>
{
  if (match(TokenType::Print)) {
    advance();  // consumme 'print'
    return print_statement();
  }
  return expr_statement();
}

auto Parser::print_statement() -> std::variant<Stmt, SyntaxError>
{
  const auto expr_opt = expression();
  if (match(TokenType::Semicolun)) {
    advance();  // just consume ';'
    return PrintStmt{as_variant<Expr>(expr_opt)};
  } else {
    return SyntaxError{SyntaxErrorKind::StmtWithoutSemicolun, peek().line, peek().column};
  }
}

auto Parser::expr_statement() -> std::variant<Stmt, SyntaxError>
{
  const auto expr_opt = expression();
  if (is_variant_v<SyntaxError>(expr_opt)) {
    return as_variant<SyntaxError>(expr_opt);
  }
  if (match(TokenType::Semicolun)) {
    advance();  // just consime ';'
    return ExprStmt{as_variant<Expr>(expr_opt)};
  }
  return SyntaxError{SyntaxErrorKind::StmtWithoutSemicolun, peek().line, peek().column};
}

auto Parser::expression() -> std::variant<Expr, SyntaxError>
{
  return equality();
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
  return primary();
}

auto Parser::primary() -> std::variant<Expr, SyntaxError>
{
  if (match(
        TokenType::Number, TokenType::String, TokenType::True, TokenType::False, TokenType::Nil)) {
    const auto & token = advance();
    return Literal{token.type, token.lexeme, token.line, token.column};
  }
  if (match(TokenType::Identifier)) {
    const auto & token = advance();
    return Variable{token};
  }
  if (match(TokenType::LeftParen)) {
    const auto left_anchor = peek();
    advance();  // just consume '('
    const auto expr_opt = expression();
    if (is_variant_v<SyntaxError>(expr_opt)) {
      return expr_opt;
    }
    if (match(TokenType::RightParen)) {
      advance();  // just consume ')'
      return Group{as_variant<Expr>(expr_opt)};
    }
    return SyntaxError{SyntaxErrorKind::UnmatchedParenError, left_anchor.line, left_anchor.column};
  }
  return SyntaxError{SyntaxErrorKind::InvalidLiteralError, peek().line, peek().column};
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

}  // namespace parser
}  // namespace lox
