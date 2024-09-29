#pragma once
#include <cpplox/expression.hpp>

#include <boost/unordered/unordered_flat_map.hpp>

#include <string_view>

namespace lox
{
inline namespace environment
{
class Environment
{
public:
  Environment() = default;

  auto define(const Token & var, const Value & var_value) -> void
  {
    values_[var.lexeme] = var_value;
  }

  auto get(const Token & name) const -> std::variant<Value, RuntimeError>
  {
    if (const auto it = values_.find(name.lexeme); it != values_.end()) {
      return it->second;
    } else {
      return RuntimeError{RuntimeErrorKind::UndefinedVariable};
    }
  }

private:
  boost::unordered_flat_map<std::string_view, Value> values_;
};

}  // namespace environment
}  // namespace lox
