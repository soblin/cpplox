#pragma once

#include <cpplox/expression.hpp>
#include <cpplox/token.hpp>

namespace lox
{
inline namespace parser
{
class Parser
{
public:
  explicit Parser(const Tokens & tokens);

  /**
    @brief <expression> ::= <equality>
   */
  auto expression() -> std::optional<Expr>;

private:
  Tokens tokens_;
  size_t current_{0};

  /**
    @brief <equality> ::= <comparison> (("==" | "!=") <comparison>)*
   */
  auto equality() -> std::optional<Expr>;

  /**
   * @brief <comparison> ::= <term> ((">" | "<" | ">=" | "<=") <term>)*
   */
  auto comparison() -> std::optional<Expr>;

  /**
   * @brief <term> ::= <factor> (("-" | "+") <factor>)*
   */
  auto term() -> std::optional<Expr>;

  /**
   * @brief <factor> ::= <unary> (("/" | "*") <unary>)*
   */
  auto factor() -> std::optional<Expr>;

  /**
   * @brief <unary> ::= ("!" | "-") <unary> | <primary>
   */
  auto unary() -> std::optional<Expr>;

  /**
   * @brief <primary> ::= NUMBER | STRING | "true" | "false" | "nil" | "(" <expression> ")"
   */
  auto primary() -> std::optional<Expr>;

  template <typename... Types>
  auto match(const Types... types) const -> bool;
  template <typename TokenType, typename... TypeTail>
  auto match(const TokenType & head, const TypeTail... tail) const -> bool
  {
    const auto type = peek().type;
    if (type == head) {
      return true;
    }
    return match<TypeTail...>(tail...);
  }
  template <>
  auto match(const TokenType & token) const -> bool
  {
    return peek().type == token;
  }

  /**
    @brief check if current token is at EOF
   */
  auto is_at_end() const noexcept -> bool;

  /**
    @brief get current token
   */
  auto peek() const -> const Token &;

  /**
    @brief increment current
   */
  auto advance() -> const Token &;
};

}  // namespace parser
}  // namespace lox
