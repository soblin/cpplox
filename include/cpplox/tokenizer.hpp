#pragma once
#include <cpplox/error.hpp>
#include <cpplox/token.hpp>

#include <optional>
#include <string>
#include <variant>

namespace lox
{

inline namespace tokenizer
{

class Tokenizer
{
public:
  explicit Tokenizer(const std::string & source);

  auto is_at_end() const noexcept -> bool;

  auto take_tokens() -> std::variant<Tokens, SyntaxError>;

private:
  const std::string source_;
  Tokens tokens_;

  auto advance() noexcept -> std::optional<char>;
  auto add_token(const TokenType & token_type) -> void;
  auto scan_new_token() -> std::optional<SyntaxError>;
  auto match(const char expected) noexcept -> bool;
  auto peek() noexcept -> char;
  auto add_string_token() -> std::optional<SyntaxError>;
  auto add_number_token() -> std::optional<SyntaxError>;
  auto peek_next() const noexcept -> char;
  auto add_identifier_token() -> std::optional<SyntaxError>;
  auto handle_newline() -> void;
  auto advance_cursor() -> void;
  auto get_token_start_column() const noexcept -> size_t;

  size_t start_{0};
  size_t current_{0};
  size_t line_{1};
  size_t column_{0};
};

auto is_digit(const char c) -> bool
{
  return c >= '0' and c <= '9';
}

auto is_alpha(const char c) -> bool
{
  return (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or c == '_';
}

}  // namespace tokenizer
}  // namespace lox
