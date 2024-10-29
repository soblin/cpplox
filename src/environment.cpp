#include <cpplox/environment.hpp>

namespace lox
{

inline namespace environment
{

/*
auto Environment::assign(const Token & var, const Value & var_value) -> std::optional<RuntimeError>
{
if (const auto it = values_.find(var.lexeme); it != values_.end()) {
  values_[var.lexeme] = var_value;
  return std::nullopt;
}
if (enclosing_) {
  return enclosing_->assign(var, var_value);
}
return UndefinedVariableError{var, Literal{var.type, var.lexeme, var.line, var.start_index}};
}
*/

auto Environment::assign_deBruijn(const Token & var, const Value & var_value, const size_t depth)
  -> std::optional<RuntimeError>
{
  if (depth == 0) {
    if (const auto it = values_.find(var.lexeme); it != values_.end()) {
      values_[var.lexeme] = var_value;
      return std::nullopt;
    }
    return UndefinedVariableError{var, Literal{var.type, var.lexeme, var.line, var.start_index}};
  }
  if (!enclosing_) {
    return UndefinedVariableError{var, Literal{var.type, var.lexeme, var.line, var.start_index}};
  }
  return enclosing_->assign_deBruijn(var, var_value, depth - 1);
}

/*
auto Environment::get(const Token & name) const -> std::variant<Value, RuntimeError>
{
if (const auto it = values_.find(name.lexeme); it != values_.end()) {
  return it->second;
} else if (enclosing_) {
  return enclosing_->get(name);
} else {
  return UndefinedVariableError{
    name, Literal{name.type, name.lexeme, name.line, name.start_index}};
}
}
*/

auto Environment::get_deBruijn(const Token & name, const size_t depth) const
  -> std::variant<Value, RuntimeError>
{
  if (depth == 0) {
    if (const auto it = values_.find(name.lexeme); it != values_.end()) {
      return it->second;
    } else {
      return UndefinedVariableError{
        name, Literal{name.type, name.lexeme, name.line, name.start_index}};
    }
  }
  if (!enclosing_) {
    return UndefinedVariableError{
      name, Literal{name.type, name.lexeme, name.line, name.start_index}};
  }
  return enclosing_->get_deBruijn(name, depth - 1);
}

}  // namespace environment
}  // namespace lox
