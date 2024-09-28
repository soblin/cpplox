#pragma once

#include <cpplox/expression.hpp>

#include <string>

namespace lox
{

inline namespace debug
{
/**
 * @brief convert Binary('1', '+', '2') to list-style "(+ 1 2)"
 */
auto to_lisp_repr(const Expr & expr) -> std::string;

}  // namespace debug
}  // namespace lox
