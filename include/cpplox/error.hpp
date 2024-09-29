#pragma once

#include <cpplox/position.hpp>
#include <cpplox/token.hpp>

#include <memory>

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
  SyntaxError(
    const SyntaxErrorKind kind, const std::shared_ptr<Line> line, const size_t ctx_start_index,
    const size_t ctx_end_index)
  : kind(kind), line(line), ctx_start_index(ctx_start_index), ctx_end_index(ctx_end_index)
  {
  }

  /**
   * @brief get the column number on its line, starting from 1
   */
  auto get_lexical_column() const noexcept -> size_t
  {
    return ctx_start_index - line->start_index + 1;
  }

  const SyntaxErrorKind kind;
  const std::shared_ptr<Line> line;
  const size_t ctx_start_index;  //<! start index of the rough range of the error
  const size_t ctx_end_index;    //<! end index of the rough range of the error
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
