#pragma once

#include <cpplox/error.hpp>
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

static constexpr const char * Bold = "\033[1m";
static constexpr const char * Italic = "\033[3m";
static constexpr const char * Thin = "\033[2m";
static constexpr const char * Underline = "\033[4m";

static constexpr const char * Red = "\033[31m";
static constexpr const char * Green = "\033[32m";
static constexpr const char * Yellow = "\033[33m";
static constexpr const char * Blue = "\033[34m";
static constexpr const char * Magenta = "\033[35m";
static constexpr const char * Cyan = "\033[36m";

static constexpr const char * Reset = "\033[0m";

}  // namespace debug
}  // namespace lox
