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
  auto declaration() -> std::variant<Declaration, SyntaxError>;

  /**
   * @brief <var_decl> := "var" IDENTIFIER ("=" <expression>)? ";"
   * @detail responsible for consuming from "var" to ';'
   */
  auto var_decl() -> std::variant<VarDecl, SyntaxError>;

  /**
   * @brief <statement> := <expr_stmt> | <print_stmt> | <block> | <if_block>
   */
  auto statement() -> std::variant<Stmt, SyntaxError>;

  /**
   * @brief <expr_stmt> := <expression> ";"
   */
  auto expr_statement() -> std::variant<ExprStmt, SyntaxError>;

  /**
   * @brief <print_stmt> := "print" <expression> ";"
   * @post after success, current_ is next to the last ';'
   */
  auto print_statement() -> std::variant<PrintStmt, SyntaxError>;

  /**
   * @brief <block> := "{" <declarations>* "}";
   * @detail responsible for consuming "{ <body> }"
   * @post after success, current_ is next to the last '}'
   */
  auto block() -> std::variant<Block, SyntaxError>;

  /**
   * @brief <if_block> := "if" "(" (<var_decl>;)? <expression> ")" "{" <declaration>* "}"
   *                      ("else if" "(" (<var_decl>;)? <expression> ")" "{" <declaration>* "}")*
   *                      ("else" "{" <declaration>* "}")?
   * @detail responsible for consuming "if () {} else if (){}... else {}"
   */
  auto if_block(const size_t if_start_ctx) -> std::variant<IfBlock, SyntaxError>;

  /**
   * @brief <while_stmt> := "while" "(" <expression> ")" "{" <declarations>* "}"
   */
  auto while_stmt(const size_t while_start_ctx) -> std::variant<WhileStmt, SyntaxError>;

  /**
   * @brief parse (...) {...}
   * @post after success, current_ is next to the last '}'
   * @detail responsible for consuming "( <cond> ){ <body> }"
   */
  auto branch_clause(const size_t if_start_ctx) -> std::variant<BranchClause, SyntaxError>;

  /**
    @brief <expression> ::= <assignment>
   */
  auto expression() -> std::variant<Expr, SyntaxError>;

  /**
    @brief <assignment> ::= IDENTIFIER "=" <assignment> | <logic_or>
   */
  auto assignment() -> std::variant<Expr, SyntaxError>;

  /**
   * @brief <logic_or> ::= <logic_and> ( "or" <logic_and> )*;
   */
  auto logic_or() -> std::variant<Expr, SyntaxError>;

  /**
   * @brief <logic_and> ::= <equality> ( "and" <equality> )*;
   */
  auto logic_and() -> std::variant<Expr, SyntaxError>;

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

  auto create_error(const SyntaxErrorKind & kind, const size_t error_ctx) const -> SyntaxError;
};

template <>
auto Parser::match(const TokenType & token) const -> bool
{
  return peek().type == token;
}

}  // namespace parser
}  // namespace lox
