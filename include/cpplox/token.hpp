#pragma once

#include <cpplox/position.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace lox
{

inline namespace token
{

enum class TokenType {
  // single token
  LeftParen,   // (
  RightParen,  // )
  LeftBrace,   // {
  RightBrace,  // }
  Comma,       // ,
  Dot,         // .
  Minus,       // -
  Plus,        // +
  Semicolun,   // ;
  Slash,       // /
  Star,        // *

  // single/double token
  Bang,          // !
  BangEqual,     // !=
  Equal,         // =
  EqualEqual,    // ==
  Greater,       // >
  GreaterEqual,  // >=
  Less,          // <
  LessEqual,     // <=

  // literal
  Identifier,
  String,
  Number,

  // keyword
  And,
  Class,
  Else,
  False,
  Fun,
  For,
  If,
  Nil,
  Or,
  Print,
  Return,
  Super,
  This,
  True,
  Var,
  While,

  Eof,
};

// clang-format off
static const std::unordered_map<std::string_view, TokenType> keyword_map = {
  {"and", TokenType::And},
  {"class", TokenType::Class},
  {"else", TokenType::Else},
  {"false", TokenType::False},
  {"fun", TokenType::Fun},
  {"for", TokenType::For},
  {"if", TokenType::If},
  {"nil", TokenType::Nil},
  {"or", TokenType::Or},
  {"print", TokenType::Print},
  {"return", TokenType::Return},
  {"super", TokenType::Super},
  {"this", TokenType::This},
  {"true", TokenType::True},
  {"var", TokenType::Var},
  {"while", TokenType::While},
};

// clang-format on

inline auto is_keyword(const std::string_view & str) -> bool
{
  return keyword_map.find(str) != keyword_map.end();
}

class Token
{
public:
  Token(
    const TokenType type, const std::string_view & lexeme, const std::shared_ptr<Line> line,
    const size_t start_index)
  : type(type), lexeme(lexeme), line(line), start_index(start_index)
  {
  }

  Token(const Token & other)
  : type(other.type), lexeme(other.lexeme), line(other.line), start_index(other.start_index)
  {
  }

  /**
   * @brief get the column number on its line, starting from 1
   */
  auto get_lexical_column() const noexcept -> size_t { return start_index - line->start_index + 1; }

  const TokenType type;
  const std::string_view lexeme;  //<! lexeme has the length information
  const std::shared_ptr<Line> line;
  const size_t start_index;  //!< the index of lexeme[0]
};

using Tokens = std::vector<Token>;

}  // namespace token

}  // namespace lox
