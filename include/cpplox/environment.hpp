#pragma once
#include <cpplox/expression.hpp>

#include <boost/unordered/unordered_flat_map.hpp>

#include <string>

namespace lox
{
inline namespace environment
{
class Environment
{
public:
  Environment() = default;
  auto define(const std::string & var_name, const Value & var_value) -> void
  {
    values_[var_name] = var_value;
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
  boost::unordered_flat_map<std::string, Value> values_;
};

}  // namespace environment
}  // namespace lox
