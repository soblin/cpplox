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

auto Parser::expression() -> std::variant<Expr, ParseError>
{
  return equality();
}

auto Parser::equality() -> std::variant<Expr, ParseError>
{
  const auto expr_opt = comparison();
  if (is_variant_v<ParseError>(expr_opt)) {
    return expr_opt;
  }
  std::vector<Expr> exprs{as_variant<Expr>(expr_opt)};
  while (match(TokenType::BangEqual, TokenType::EqualEqual)) {
    const auto & op = advance();
    const auto right_opt = comparison();
    if (is_variant_v<ParseError>(right_opt)) {
      return right_opt;
    }
    const auto binary = Binary{exprs.back(), op, as_variant<Expr>(right_opt)};
    exprs.push_back(binary);
  }
  return exprs.back();
}

auto Parser::comparison() -> std::variant<Expr, ParseError>
{
  const auto expr_opt = term();
  if (is_variant_v<ParseError>(expr_opt)) {
    return expr_opt;
  }
  std::vector<Expr> exprs{as_variant<Expr>(expr_opt)};
  while (
    match(TokenType::Greater, TokenType::GreaterEqual, TokenType::Less, TokenType::LessEqual)) {
    const auto & op = advance();
    const auto right_opt = term();
    if (is_variant_v<ParseError>(right_opt)) {
      return right_opt;
    }
    const auto binary = Binary{exprs.back(), op, as_variant<Expr>(right_opt)};
    exprs.push_back(binary);
  }
  return exprs.back();
}

auto Parser::term() -> std::variant<Expr, ParseError>
{
  const auto factor_opt = factor();
  if (is_variant_v<ParseError>(factor_opt)) {
    return factor_opt;
  }
  std::vector<Expr> exprs{as_variant<Expr>(factor_opt)};
  while (match(TokenType::Plus, TokenType::Minus)) {
    const auto & op = advance();
    const auto right_opt = factor();
    if (is_variant_v<ParseError>(right_opt)) {
      return right_opt;
    }
    const auto binary = Binary{exprs.back(), op, as_variant<Expr>(right_opt)};
    exprs.push_back(binary);
  }
  return exprs.back();
}

auto Parser::factor() -> std::variant<Expr, ParseError>
{
  const auto unary_opt = unary();
  if (is_variant_v<ParseError>(unary_opt)) {
    return unary_opt;
  }
  std::vector<Expr> exprs{as_variant<Expr>(unary_opt)};
  while (match(TokenType::Slash, TokenType::Star)) {
    const auto & op = advance();
    const auto right_opt = unary();
    if (is_variant_v<ParseError>(right_opt)) {
      return right_opt;
    }
    const auto binary = Binary{exprs.back(), op, as_variant<Expr>(right_opt)};
    exprs.push_back(binary);
  }
  return exprs.back();
}

auto Parser::unary() -> std::variant<Expr, ParseError>
{
  if (match(TokenType::Bang, TokenType::Minus)) {
    const auto & op = advance();
    const auto unary_next_opt = unary();
    if (is_variant_v<ParseError>(unary_next_opt)) {
      return unary_next_opt;
    }
    return Unary{op, as_variant<Expr>(unary_next_opt)};
  }
  return primary();
}

auto Parser::primary() -> std::variant<Expr, ParseError>
{
  if (match(
        TokenType::Number, TokenType::String, TokenType::True, TokenType::False, TokenType::Nil)) {
    const auto & token = advance();
    return Literal{token.type, token.lexeme, token.line, token.column};
  }
  if (match(TokenType::LeftParen)) {
    const auto left_anchor = peek();
    advance();  // just consume '('
    const auto expr_opt = expression();
    if (is_variant_v<ParseError>(expr_opt)) {
      return expr_opt;
    }
    if (match(TokenType::RightParen)) {
      advance();  // just consume ')'
      return Group{as_variant<Expr>(expr_opt)};
    }
    return ParseError{ParseErrorKind::UnmatchedParenError, left_anchor.line, left_anchor.column};
  }
  return ParseError{ParseErrorKind::InvalidLiteralError, peek().line, peek().column};
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
