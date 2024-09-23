#pragma once
#include <cpplox/error.hpp>
#include <cpplox/token.hpp>

#include <boost/variant/recursive_variant.hpp>

#include <string>

namespace lox
{

inline namespace expression
{

struct Literal;
struct Unary;
struct Binary;
struct Group;

using Expr = boost::variant<
  Literal, boost::recursive_wrapper<Unary>, boost::recursive_wrapper<Binary>,
  boost::recursive_wrapper<Group>>;

struct Literal : public Token
{
  using Token::Token;
};

struct Unary
{
  const Token op;
  const Expr expr;
};

struct Binary
{
  const Expr left;
  const Token op;
  const Expr right;
};

struct Group
{
  const Expr expr;
};

/**
 * @brief convert Binary('1', '+', '2') to list-style "(+ 1 2)"
 */
auto to_lisp_repr(const Expr & expr) -> std::string;

using Nil = std::monostate;
using Value = std::variant<Nil, bool, double, std::string>;

auto is_truthy(const Value & value) -> bool;

auto is_equal(const Value & left, const Value & right) -> bool;

auto evaluate_expr(const Expr & expr) -> std::variant<Value, InterpretError>;

}  // namespace expression
}  // namespace lox
