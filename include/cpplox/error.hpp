#pragma once

#include <cpplox/token.hpp>

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
  StmtWithoutSemicolun,        // statement does not end with ';'
  MissingValidIdentifierDecl,  // variable declaration without valid identifier
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

enum class RuntimeErrorKind {
  TypeError,  // 1 + "2"
};

struct RuntimeError
{
  const RuntimeErrorKind kind;
};

}  // namespace error
}  // namespace lox
