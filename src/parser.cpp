#include <cpplox/parser.hpp>

#include <cassert>

namespace lox
{

inline namespace parser
{

static int a = 123;
static int b = 123;

Parser::Parser(const Tokens & tokens) : tokens_(tokens)
{
  if (tokens_.empty() or tokens_.back().type != TokenType::Eof) {
    tokens_.emplace_back(TokenType::Eof, "<EOF>", 0);
  }
  assert(tokens_.size() >= 1);
  int a = 123;
}

auto Parser::expression() -> std::optional<Expr>
{
  return equality();
}

auto Parser::equality() -> std::optional<Expr>
{
  const auto expr_opt = comparison();
  if (!expr_opt) {
    return std::nullopt;
  }
  std::vector<Expr> exprs{expr_opt.value()};
  while (match(TokenType::BangEqual, TokenType::EqualEqual)) {
    const auto & op = advance();
    const auto right_opt = comparison();
    if (!right_opt) {
      return std::nullopt;
    }
    const auto binary = Binary{exprs.back(), op, right_opt.value()};
    exprs.push_back(binary);
  }
  return exprs.back();
}

auto Parser::comparison() -> std::optional<Expr>
{
  const auto expr_opt = term();
  if (!expr_opt) {
    return std::nullopt;
  }
  std::vector<Expr> exprs{expr_opt.value()};
  while (
    match(TokenType::Greater, TokenType::GreaterEqual, TokenType::Less, TokenType::LessEqual)) {
    const auto & op = advance();
    const auto right_opt = term();
    if (!right_opt) {
      return std::nullopt;
    }
    const auto binary = Binary{exprs.back(), op, right_opt.value()};
    exprs.push_back(binary);
  }
  return exprs.back();
}

auto Parser::term() -> std::optional<Expr>
{
  const auto factor_opt = factor();
  if (!factor_opt) {
    return std::nullopt;
  }
  std::vector<Expr> exprs{factor_opt.value()};
  while (match(TokenType::Plus, TokenType::Minus)) {
    const auto & op = advance();
    const auto right_opt = factor();
    if (!right_opt) {
      return std::nullopt;
    }
    const auto binary = Binary{exprs.back(), op, right_opt.value()};
    exprs.push_back(binary);
  }
  return exprs.back();
}

auto Parser::factor() -> std::optional<Expr>
{
  const auto unary_opt = unary();
  if (!unary_opt) {
    return std::nullopt;
  }
  std::vector<Expr> exprs{unary_opt.value()};
  while (match(TokenType::Slash, TokenType::Star)) {
    const auto & op = advance();
    const auto right_opt = unary();
    if (!right_opt) {
      return std::nullopt;
    }
    const auto binary = Binary{exprs.back(), op, right_opt.value()};
    exprs.push_back(binary);
  }
  return exprs.back();
}

auto Parser::unary() -> std::optional<Expr>
{
  if (match(TokenType::Bang, TokenType::Minus)) {
    const auto & op = advance();
    const auto unary_next_opt = unary();
    if (!unary_next_opt) {
      return std::nullopt;
    }
    return Unary{op, unary_next_opt.value()};
  }
  return primary();
}

auto Parser::primary() -> std::optional<Expr>
{
  if (match(
        TokenType::Number, TokenType::String, TokenType::True, TokenType::False, TokenType::Nil)) {
    const auto & token = advance();
    return Literal{token.type, token.lexeme, token.line};
  }
  if (match(TokenType::LeftParen)) {
    advance();  // just consume '('
    const auto expr_opt = expression();
    if (!expr_opt) {
      return std::nullopt;
    }
    if (match(TokenType::RightParen)) {
      advance();  // just consume ')'
      return Group{expr_opt.value()};
    }
    return std::nullopt;
  }
  return std::nullopt;
}

auto Parser::is_at_end() const noexcept -> bool
{
  if (current_ == tokens_.size()) {
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
