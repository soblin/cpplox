#pragma once

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

struct InvalidCharacterError
{
  InvalidCharacterError(const char character, const size_t line, const size_t row = 0)
  : character(character), line(line), row(row)
  {
  }
  const char character;
  const size_t line;
  const size_t row;
};

struct NonTerminatedStringError
{
  NonTerminatedStringError(const std::string & str, const size_t line, const size_t row = 0)
  : str(str), line(line), row(row)
  {
  }
  const std::string str;
  const size_t line;
  const size_t row;
};

struct NonTerminatedNumberError
{
  NonTerminatedNumberError(const std::string & str, const size_t line, const size_t row = 0)
  : str(str), line(line), row(row)
  {
  }
  const std::string str;
  const size_t line;
  const size_t row;
};

struct InvalidNumberError
{
  InvalidNumberError(const std::string & str, const size_t line, const size_t row = 0)
  : str(str), line(line), row(row)
  {
  }
  const std::string str;
  const size_t line;
  const size_t row;
};

struct InvalidIdentifierError
{
  InvalidIdentifierError(const std::string & str, const size_t line, const size_t row = 0)
  : str(str), line(line), row(row)
  {
  }
  const std::string str;
  const size_t line;
  const size_t row;
};

using ParseError = std::variant<
  InvalidCharacterError,     // unsupported character like '@'
  NonTerminatedStringError,  // '"' does not end
  NonTerminatedNumberError,  // "123<EOF>"
  InvalidNumberError,        // "123.a", "123abc"
  InvalidIdentifierError>;

auto format_parse_error(const ParseError & error) -> std::string;

}  // namespace error
}  // namespace lox
