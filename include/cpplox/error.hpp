#pragma once

#include <cpplox/token.hpp>

namespace lox
{
inline namespace error
{

enum class SyntaxErrorKind {
  InvalidCharacterError,       //!< unsupported character like '@'
  NonTerminatedStringError,    //!< '"' does not end
  InvalidNumberError,          //!< "123.a", "123abc"
  InvalidLiteralError,         //!<
  UnmatchedParenError,         //!< "(1 + 2"
  StmtWithoutSemicolun,        //!< statement does not end with ';'
  MissingValidIdentifierDecl,  //!< variable declaration without valid identifier
  MissingAssignmentOperator,   //!< "foo 3" is interpreted as '=' is missing in "foo = 3";
  InvalidAssignmentTarget,     //!< like "1 + 2 = 10;"
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
  TypeError,          //!< 1 + "2"
  UndefinedVariable,  //!<
};

struct RuntimeError
{
  const RuntimeErrorKind kind;
};

}  // namespace error
}  // namespace lox
