#pragma once
#include <cpplox/expression.hpp>

#include <boost/unordered/unordered_flat_map.hpp>

#include <memory>
#include <string>

namespace lox
{
inline namespace environment
{
class Environment
{
public:
  Environment() = default;

  explicit Environment(std::shared_ptr<Environment> enclosing) : enclosing_(enclosing) {}

  auto define(const Token & var, const Value & var_value) -> void
  {
    values_[std::string(var.lexeme)] = var_value;
  }

  [[nodiscard]] auto assign(const Token & var, const Value & var_value)
    -> std::optional<RuntimeError>
  {
    const auto key = std::string(var.lexeme);
    if (const auto it = values_.find(key); it != values_.end()) {
      values_[key] = var_value;
      return std::nullopt;
    }
    if (enclosing_) {
      return enclosing_->assign(var, var_value);
    }
    return RuntimeError{RuntimeErrorKind::UndefinedVariable};
  }

  auto get(const Token & name) const -> std::variant<Value, RuntimeError>
  {
    if (const auto it = values_.find(std::string(name.lexeme)); it != values_.end()) {
      return it->second;
    } else if (enclosing_) {
      return enclosing_->get(name);
    } else {
      return RuntimeError{RuntimeErrorKind::UndefinedVariable};
    }
  }

private:
  // NOTE: if the key is string_view, even if the string content is same "foo", if the source of the
  // string_view is different like in the REPL mode, "foo" will be treated as different
  boost::unordered_flat_map<std::string, Value> values_;

  /**
   * @brief when a sub scope is created, the sub-environment has the main scope as "enclosing".
   * "enclosing" is nullptr if and only if the enviroment is global scope
   */
  std::shared_ptr<Environment> enclosing_{nullptr};
};

}  // namespace environment
}  // namespace lox
