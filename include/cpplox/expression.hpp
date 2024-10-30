#pragma once
#include <cpplox/token.hpp>
#include <cpplox/variant.hpp>

#include <boost/variant/recursive_variant.hpp>

#include <memory>
#include <string>
#include <vector>

namespace lox
{

inline namespace stmt
{
struct FuncDecl;
}

inline namespace environment
{
class Environment;
}

inline namespace expression
{

struct Literal;
struct Unary;
struct Binary;
struct Group;
struct Variable;
struct Assign;
struct Logical;
struct Call;

using Expr = boost::variant<
  Literal, boost::recursive_wrapper<Unary>, boost::recursive_wrapper<Binary>,
  boost::recursive_wrapper<Group>, boost::recursive_wrapper<Variable>,
  boost::recursive_wrapper<Assign>, boost::recursive_wrapper<Logical>,
  boost::recursive_wrapper<Call>>;

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

struct Call
{
  const Expr callee;
  const std::vector<Expr> arguments;
};

using Nil = std::monostate;

struct Callable
{
  std::shared_ptr<const FuncDecl> definition;
  /**
   * NOTE: when a closure is defined in a subscope, the subscope owns this Callable object, and the
   * closure object itself refers to the subscope at `closure` field. Thus `closure` field needs to
   * be weak_ptr because otherwise they form a circular dependency.
   */
  std::shared_ptr<Environment> closure;
};

struct Class
{
  const Token name;
};

using Value = boost::variant<Nil, bool, int64_t, double, std::string, Callable, Class>;

namespace helper
{
constexpr size_t Nil_Index = 0;
constexpr size_t bool_Index = 1;
constexpr size_t long_Index = 2;
constexpr size_t double_Index = 3;
constexpr size_t str_Index = 4;

inline auto is_nil(const Value & value) -> bool
{
  return is_variant_v<Nil>(value);
}

inline auto is_bool(const Value & value) -> bool
{
  return is_variant_v<bool>(value);
}

inline auto is_long(const Value & value) -> bool
{
  return is_variant_v<int64_t>(value);
}

inline auto is_double(const Value & value) -> bool
{
  return is_variant_v<double>(value);
}

inline auto is_numeric(const Value & value) -> bool
{
  return is_long(value) || is_double(value);
}

inline auto is_str(const Value & value) -> bool
{
  return is_variant_v<std::string>(value);
}

};  // namespace helper

auto is_truthy(const Value & value) -> bool;

auto is_equal(const Value & left, const Value & right) -> bool;

}  // namespace expression
}  // namespace lox
