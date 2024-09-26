#include <cpplox/expression.hpp>
#include <cpplox/interpreter.hpp>

#include <iostream>

namespace lox
{

inline namespace interpreter
{

auto Interpreter::execute(const std::vector<Stmt> & program) -> std::optional<RuntimeError>
{
  auto stringify = [](const Value & value) -> std::string {
    return std::visit(
      visit_variant{
        [](const Nil & nil) -> std::string { return "nil"; },
        [](const bool & boolean) -> std::string { return boolean ? "true" : "false"; },
        [](const int64_t & i) -> std::string { return std::to_string(i); },
        [](const double & d) -> std::string { return std::to_string(d); },
        [](const std::string & str) -> std::string { return str; },
      },
      value);
  };
  for (const auto & statement : program) {
    const std::optional<RuntimeError> result = std::visit(
      visit_variant{
        [&](const ExprStmt & stmt) -> std::optional<RuntimeError> {
          const auto eval_opt = evaluate_expr(stmt.expression);
          if (is_variant_v<RuntimeError>(eval_opt)) {
            std::cerr << lox::to_lisp_repr(stmt.expression) << std::endl;
            return as_variant<RuntimeError>(eval_opt);
          }
          std::cout << "(for debug: value is " << stringify(as_variant<Value>(eval_opt)) << ")"
                    << std::endl;
          return std::nullopt;
        },
        [&](const PrintStmt & stmt) -> std::optional<RuntimeError> {
          const auto eval_opt = evaluate_expr(stmt.expression);
          if (is_variant_v<RuntimeError>(eval_opt)) {
            std::cerr << lox::to_lisp_repr(stmt.expression) << std::endl;
            return as_variant<RuntimeError>(eval_opt);
          }
          std::cout << "print " << stringify(as_variant<Value>(eval_opt)) << std::endl;
          return std::nullopt;
        },
      },
      statement);
    if (result) {
      return result.value();
    }
  }
  return std::nullopt;
}

}  // namespace interpreter
}  // namespace lox
