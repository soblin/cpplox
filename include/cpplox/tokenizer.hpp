#pragma once
#include <cpplox/error.hpp>
#include <cpplox/token.hpp>

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace lox
{

inline namespace tokenizer
{

class Tokenizer
{
public:
  explicit Tokenizer(const std::string & source);

  auto is_at_end() const noexcept -> bool;

  /**
   * @brief return the internal stored tokens to caller
   * @post internal stored tokens get empty
   */
  auto take_tokens() -> std::variant<Tokens, SyntaxError>;

private:
  const std::string & source_;
  Tokens tokens_;
  std::vector<std::shared_ptr<Line>> lines_;

  /**
   * @brief get current peek() character and then increment the cursor
   * @post current cursor and current column is incremented
   */
  auto advance() noexcept -> std::optional<char>;

  auto add_token(const TokenType & token_type) -> void;

  /**
   * @brief add a new token
   * @return nullopt if successful, SyntaxError value if unsuccessful
   * @post if successful, current cursor points to the position right after the newly added token
   */
  auto scan_new_token() -> std::optional<SyntaxError>;

  /**
   * @brief check if current peek() matches given characater, no consumption
   */
  auto match(const char expected) noexcept -> bool;

  /**
   * @brief get current cursor character, no consumption
   */
  auto peek() noexcept -> char;

  /**
   * @brief get next cursor character, no consumption. If the cursor is at end, returns '\0'
   */
  auto peek_next() const noexcept -> char;

  /**
   * @brief when '"' is found, get the enclosed part of the string
   * @note escape sequence is not supported
   * @post if successful, current cursor points to the position right after the '"'
   */
  auto add_string_token() -> std::optional<SyntaxError>;

  /**
   * @brief when a beginning digit is found, get the remaining part of the number includeing the '.'
   * @return if the token cannot be interpreted as a int64_t/double, return SyntaxError
   * @post if successful, current cursor points to the position right after the last digit
   */
  auto add_number_token() -> std::optional<SyntaxError>;

  auto add_identifier_token() -> std::optional<SyntaxError>;

  /**
   * @brief increment line number and reset column number
   */
  auto handle_newline() -> void;

  /**
   * @brief increment current cursor and current column
   */
  auto advance_cursor() -> void;

  /**
   * @brief get the column number of the start position of current token
   */
  auto get_token_start_column() const noexcept -> size_t;

  auto get_next_newline_or_eof() const noexcept -> size_t;

  auto create_error(const SyntaxErrorKind kind) -> SyntaxError;

  size_t current_ctx_start_cursor_{0};  //!< save the start of a token while scanning the token
                                        //!< to the end
  size_t current_cursor_{0};            //!< the current position of scanner
  size_t line_{1};  //!< the current line number (number of '\n' so far) starting from 1
  size_t current_line_start_index_{0};  //!< the start index of current line
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
