#pragma once
#include <cpplox/error.hpp>
#include <cpplox/expression.hpp>

#include <memory>
#include <string>
#include <unordered_map>

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
    values_[var.lexeme] = var_value;
  }

  /*
  [[nodiscard]] auto assign(const Token & var, const Value & var_value)
    -> std::optional<RuntimeError>;
  auto get(const Token & name) const -> std::variant<Value, RuntimeError>;
  */

  [[nodiscard]] auto assign_deBruijn(const Token & var, const Value & var_value, const size_t depth)
    -> std::optional<RuntimeError>;

  auto get_deBruijn(const Token & name, const size_t depth) const
    -> std::variant<Value, RuntimeError>;

private:
  std::unordered_map<std::string_view, Value> values_;

  /**
   * @brief when a sub scope is created, the sub-environment has the main scope as "enclosing".
   * "enclosing" is nullptr if and only if the enviroment is global scope
   */
  std::shared_ptr<Environment> enclosing_{nullptr};
};

}  // namespace environment
}  // namespace lox
