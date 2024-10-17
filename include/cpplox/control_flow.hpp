#pragma once
#include <cpplox/expression.hpp>

#include <variant>

namespace lox
{

inline namespace flow
{

struct Break
{
};

struct Continue
{
};

struct Return
{
  const Value value;
};

using ControlFlowKind = std::variant<Break, Continue, Return>;

}  // namespace flow
}  // namespace lox
