#include <cpplox/environment.hpp>

namespace lox
{

inline namespace environment
{

auto Environment::assign(const Token & var, const Value & var_value) -> std::optional<RuntimeError>
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

auto Environment::get(const Token & name) const -> std::variant<Value, RuntimeError>
{
  if (const auto it = values_.find(std::string(name.lexeme)); it != values_.end()) {
    return it->second;
  } else if (enclosing_) {
    return enclosing_->get(name);
  } else {
    return RuntimeError{RuntimeErrorKind::UndefinedVariable};
  }
}
}  // namespace environment
}  // namespace lox
