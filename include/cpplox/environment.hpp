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

  auto define(const Token & var, const Value & var_value) -> void
  {
    values_[std::string(var.lexeme)] = var_value;
  }

  auto get(const Token & name) const -> std::variant<Value, RuntimeError>
  {
    if (const auto it = values_.find(std::string(name.lexeme)); it != values_.end()) {
      return it->second;
    } else {
      return RuntimeError{RuntimeErrorKind::UndefinedVariable};
    }
  }

private:
  // NOTE: if the key is string_view, even if the string content is same "foo", if the source of the
  // string_view is different like in the REPL mode, "foo" will be treated as different
  boost::unordered_flat_map<std::string, Value> values_;
};

}  // namespace environment
}  // namespace lox
