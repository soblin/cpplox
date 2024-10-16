#pragma once

#include <cpplox/expression.hpp>
#include <cpplox/position.hpp>
#include <cpplox/system.hpp>
#include <cpplox/token.hpp>

#include <memory>
#include <string>

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
  UnmatchedBraceError,         //!< "{a = 100;"
  MissingIfConditon,           //!< (cond) if missing like "if { <body> }}"
  MissingIfBody,               //!< "{ <body> }" is not found after "if"
  MissingWhileConditon,        //!< (cond) if missing like "while { <body> }}"
  MissingWhileBody,            //!< "{ <body> }" is not found after "while"
  MissingForCondition,         //!< (cond) if missing like "for { <body> }}"
  MissingForBody,              //!< "{ <body> }" is not found after "for"
  TooManyArguments,            //<! the number of arguments is too large
  MissingFuncParameterDecl,    //<! function declaration without parenthesis "()"
  InvalidParameterDecl,        //<! parameter is not identifier
  MissingFuncBodyDecl,         //<! function declaration lacks body
};

struct SyntaxError
{
  SyntaxError(
    const SyntaxErrorKind kind, const std::shared_ptr<Line> line, const size_t ctx_start_index,
    const size_t ctx_end_index)
  : kind(kind), line(line), ctx_start_index(ctx_start_index), ctx_end_index(ctx_end_index)
  {
  }

  SyntaxError(const SyntaxError & other)
  : kind(other.kind),
    line(other.line),
    ctx_start_index(other.ctx_start_index),
    ctx_end_index(other.ctx_end_index)
  {
  }

  /**
   * @brief get the column number on its line, starting from 1
   */
  auto get_lexical_column() const noexcept -> size_t
  {
    return ctx_start_index - line->start_index + 1;
  }

  // LCOV_EXCL_START
  auto get_line_string(const size_t offset = 0) const -> std::string;

  auto get_visualization_string(const std::string_view & source, const size_t offset = 0) const
    -> std::string;
  // LCOV_EXCL_STOP

  const SyntaxErrorKind kind;
  const std::shared_ptr<Line> line;
  const size_t ctx_start_index;  //<! start index of the rough range of the error
  const size_t ctx_end_index;    //<! end index of the rough range of the error
};

struct TypeError
{
  // 1 + "2"
  const Token op;
  const Expr expr;
};

struct UndefinedVariableError
{
  const Token variable;  //!< the variable
  const Expr expr;       //!< the entire expression
};

struct MaxLoopError
{
  static constexpr size_t Limit = max_recursion_limit;
  const Token token;               //!< the cause statement of this loop
  const std::optional<Expr> cond;  //!< the cause of max loop
};

struct NotInvocableError
{
  const Expr callee;
  const std::string desc;
};

enum class PseudoSignalKind {
  Break,
  Continue,
};

using RuntimeError =
  std::variant<TypeError, UndefinedVariableError, MaxLoopError, NotInvocableError>;

// LCOV_EXCL_START
auto get_line_string(const RuntimeError & error, const size_t offset = 0) -> std::string;

auto get_visualization_string(
  const std::string & source, const RuntimeError & error, const size_t offset = 0) -> std::string;

// LCOV_EXCL_STOP

}  // namespace error
}  // namespace lox
