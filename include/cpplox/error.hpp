#pragma once

#include <cpplox/token.hpp>

#include <string>
#include <utility>
#include <variant>

namespace lox
{
inline namespace error
{

/*
  TODO(soblin): report source file if the env is not REPL
 */

enum class SyntaxErrorKind {
  InvalidCharacterError,     // unsupported character like '@'
  NonTerminatedStringError,  // '"' does not end
  InvalidNumberError,        // "123.a", "123abc"
  // InvalidIdentifierError,
  InvalidLiteralError,
  UnmatchedParenError,
};

struct SyntaxError
{
  SyntaxError(const SyntaxErrorKind kind, const size_t line, const size_t column)
  : kind(kind), line(line), column(column)
  {
  }
  const SyntaxErrorKind kind;
  const size_t line;
  const size_t column;
};

auto format_parse_error(const SyntaxError & error) -> std::string;

enum class InterpretErrorKind {
  TypeError,  // 1 + "2"
};

struct InterpretError
{
  const InterpretErrorKind kind;
};

}  // namespace error
}  // namespace lox
