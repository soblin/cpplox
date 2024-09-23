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

enum class ParseErrorKind {
  InvalidCharacterError,     // unsupported character like '@'
  NonTerminatedStringError,  // '"' does not end
  NonTerminatedNumberError,  // "123<EOF>"
  InvalidNumberError,        // "123.a", "123abc"
  InvalidIdentifierError,
  InvalidLiteralError,
  UnmatchedParenError
};

struct ParseError
{
  ParseError(const ParseErrorKind kind, const size_t line, const size_t column)
  : kind(kind), line(line), column(column)
  {
  }
  const ParseErrorKind kind;
  const size_t line;
  const size_t column;
};

auto format_parse_error(const ParseError & error) -> std::string;

}  // namespace error
}  // namespace lox
