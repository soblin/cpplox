#pragma once
#include <cpplox/token.hpp>
#include <cpplox/variant.hpp>

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
struct Variable;
struct Assign;
struct Logical;

using Expr = boost::variant<
  Literal, boost::recursive_wrapper<Unary>, boost::recursive_wrapper<Binary>,
  boost::recursive_wrapper<Group>, boost::recursive_wrapper<Variable>,
  boost::recursive_wrapper<Assign>, boost::recursive_wrapper<Logical>>;

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
  const Token left_paren;  //!< only for saving position info
  const Expr expr;
  const Token right_paren;  //!< only for saving position info
};

struct Variable
{
  const Token name;
};

struct Assign
{
  const Token name;
  /**
     a = b = 1; is
     a = (b = 1); where (b = 1) is also "Assign"
  */
  const Expr expr;
};

struct Logical
{
  const Expr left;
  const Token op;
  const Expr right;
};

using Nil = std::monostate;
using Value = std::variant<Nil, bool, int64_t, double, std::string>;

namespace helper
{
constexpr size_t Nil_Index = 0;
constexpr size_t bool_Index = 1;
constexpr size_t long_Index = 2;
constexpr size_t double_Index = 3;
constexpr size_t str_Index = 4;

inline auto is_nil(const Value & value) -> bool
{
  return value.index() == Nil_Index;
}

inline auto is_bool(const Value & value) -> bool
{
  return value.index() == bool_Index;
}

inline auto is_long(const Value & value) -> bool
{
  return value.index() == long_Index;
}

inline auto is_double(const Value & value) -> bool
{
  return value.index() == double_Index;
}

inline auto is_numeric(const Value & value) -> bool
{
  return value.index() == long_Index || value.index() == double_Index;
}

inline auto is_str(const Value & value) -> bool
{
  return value.index() == str_Index;
}

};  // namespace helper

auto is_truthy(const Value & value) -> bool;

auto is_equal(const Value & left, const Value & right) -> bool;

}  // namespace expression
}  // namespace lox
