#pragma once

#include <cpplox/error.hpp>
#include <cpplox/expression.hpp>
#include <cpplox/statement.hpp>

#include <optional>
#include <vector>

namespace lox
{

inline namespace interpreter
{

class Interpreter
{
public:
  auto execute(const std::vector<Stmt> & program) -> std::optional<RuntimeError>;
};

}  // namespace interpreter
}  // namespace lox
