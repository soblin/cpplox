#pragma once

#include <cpplox/error.hpp>
#include <cpplox/expression.hpp>
#include <cpplox/statement.hpp>
#include <cpplox/token.hpp>

#include <vector>

namespace lox
{
inline namespace parser
{
class Parser
{
public:
  explicit Parser(const Tokens & tokens);

  /**
   * @brief <program> := <declaration>* EOF
   */
  auto program() -> std::variant<Program, SyntaxError>;

  /**
   * @brief <declaration> := <var_decl> | <statement>
   */
  auto declaration() -> std::variant<Stmt, SyntaxError>;

  /**
   * @brief <var_decl> := "var" IDENTIFIER ("=" <expression>)? ";"
   */
  auto var_decl() -> std::variant<Stmt, SyntaxError>;

  /**
   * @brief <statement> := <expr_stmt> | <print_stmt>
   */
  auto statement() -> std::variant<Stmt, SyntaxError>;

  /**
   * @brief <expr_stmt> := <expression> ";"
   */
  auto expr_statement() -> std::variant<Stmt, SyntaxError>;

  /**
   * @brief <print_stmt> := "print" <expression> ";"
   */
  auto print_statement() -> std::variant<Stmt, SyntaxError>;

  /**
    @brief <expression> ::= <equality>
   */
  auto expression() -> std::variant<Expr, SyntaxError>;

private:
  Tokens tokens_;
  size_t current_{0};

  /**
    @brief <equality> ::= <comparison> (("==" | "!=") <comparison>)*
   */
  auto equality() -> std::variant<Expr, SyntaxError>;

  /**
   * @brief <comparison> ::= <term> ((">" | "<" | ">=" | "<=") <term>)*
   */
  auto comparison() -> std::variant<Expr, SyntaxError>;

  /**
   * @brief <term> ::= <factor> (("-" | "+") <factor>)*
   */
  auto term() -> std::variant<Expr, SyntaxError>;

  /**
   * @brief <factor> ::= <unary> (("/" | "*") <unary>)*
   */
  auto factor() -> std::variant<Expr, SyntaxError>;

  /**
   * @brief <unary> ::= ("!" | "-") <unary> | <primary>
   */
  auto unary() -> std::variant<Expr, SyntaxError>;

  /**
   * @brief <primary> ::= NUMBER | STRING | "true" | "false" | "nil" | "(" <expression> ")" |
   * IDENTIFIER
   */
  auto primary() -> std::variant<Expr, SyntaxError>;

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

template <>
auto Parser::match(const TokenType & token) const -> bool
{
  return peek().type == token;
}

}  // namespace parser
}  // namespace lox
