#include <cpplox/tokenizer.hpp>
#include <cpplox/variant.hpp>

#include <boost/lexical_cast.hpp>

#include <string>

namespace lox
{

inline namespace tokenizer
{

Tokenizer::Tokenizer(const std::string & source) : source_(source)
{
  const size_t end = get_next_newline_or_eof();
  lines_.emplace_back(std::make_shared<Line>(std::nullopt, 1, current_cursor_, end));
  assert(!lines_.empty());
}

auto Tokenizer::is_at_end() const noexcept -> bool
{
  return current_cursor_ >= source_.size();
}

auto Tokenizer::take_tokens() -> std::variant<Tokens, SyntaxError>
{
  while (!is_at_end()) {
    current_ctx_start_cursor_ = current_cursor_;
    auto err_opt = scan_new_token();
    if (err_opt) {
      return err_opt.value();
    }
  }
  return std::move(tokens_);
}

auto Tokenizer::advance() noexcept -> std::optional<char>
{
  if (!is_at_end()) {
    auto c = source_.at(current_cursor_);
    advance_cursor();
    return c;
  } else {
    return std::nullopt;  // LCOV_EXCL_LINE
  }
}

auto Tokenizer::add_token(const TokenType & token_type) -> void
{
  const auto start = (token_type == TokenType::String) ? (current_ctx_start_cursor_ + 1)
                                                       : current_ctx_start_cursor_;  // to skip '"'
  const auto len = (token_type == TokenType::String)
                     ? (current_cursor_ - current_ctx_start_cursor_ - 2)
                     : (current_cursor_ - current_ctx_start_cursor_);
  const auto text = std::string_view(source_).substr(start, len);
  if (token_type == TokenType::Identifier and is_keyword(text)) {
    tokens_.emplace_back(keyword_map.find(text)->second, text, lines_.back(), start);
    return;
  }
  tokens_.emplace_back(token_type, text, lines_.back(), start);
}

auto Tokenizer::scan_new_token() -> std::optional<SyntaxError>
{
  const auto c_opt = advance();
  if (!c_opt) {
    add_token(TokenType::Eof);  // LCOV_EXCL_LINE
    return std::nullopt;        // LCOV_EXCL_LINE
  }
  const auto c = c_opt.value();
  if (c == '(') {
    add_token(TokenType::LeftParen);
    return std::nullopt;
  }
  if (c == ')') {
    add_token(TokenType::RightParen);
    return std::nullopt;
  }
  if (c == '{') {
    add_token(TokenType::LeftBrace);
    return std::nullopt;
  }
  if (c == '}') {
    add_token(TokenType::RightBrace);
    return std::nullopt;
  }
  if (c == ',') {
    add_token(TokenType::Comma);
    return std::nullopt;
  }
  if (c == '.') {
    add_token(TokenType::Dot);
    return std::nullopt;
  }
  if (c == '-') {
    add_token(TokenType::Minus);
    return std::nullopt;
  }
  if (c == '+') {
    add_token(TokenType::Plus);
    return std::nullopt;
  }
  if (c == ';') {
    add_token(TokenType::Semicolun);
    return std::nullopt;
  }
  if (c == '/') {
    if (match('/')) {
      while (peek() != '\n'    // while the comment body ends
             and !is_at_end()  // in case the source file ends with comment
      ) {
        advance();
      }
      advance();
      handle_newline();
      return std::nullopt;
    } else {
      add_token(TokenType::Slash);
      return std::nullopt;
    }
  }
  if (c == '*') {
    add_token(TokenType::Star);
    return std::nullopt;
  }
  if (c == '!') {
    add_token(match('=') ? TokenType::BangEqual : TokenType::Bang);
    return std::nullopt;
  }
  if (c == '=') {
    add_token(match('=') ? TokenType::EqualEqual : TokenType::Equal);
    return std::nullopt;
  }
  if (c == '>') {
    add_token(match('=') ? TokenType::GreaterEqual : TokenType::Greater);
    return std::nullopt;
  }
  if (c == '<') {
    add_token(match('=') ? TokenType::LessEqual : TokenType::Less);
    return std::nullopt;
  }
  if (c == ' ' or c == '\r' or c == '\t') {
    return std::nullopt;
  }
  if (c == '\n') {
    handle_newline();
    return std::nullopt;
  }
  if (c == '"') {
    auto err_opt = add_string_token();
    if (err_opt) {
      return std::move(err_opt.value());
    }
    return std::nullopt;
  }
  if (is_digit(c)) {
    auto err_opt = add_number_token();
    if (err_opt) {
      return std::move(err_opt.value());
    }
    return std::nullopt;
  }
  if (is_alpha(c)) {
    auto err_opt = add_identifier_token();
    if (err_opt) {
      return std::move(err_opt.value());
    }
    return std::nullopt;
  }
  return create_error(SyntaxErrorKind::InvalidCharacterError);
}

auto Tokenizer::match(const char expected) noexcept -> bool
{
  if (is_at_end()) {
    return false;
  }
  if (source_.at(current_cursor_) != expected) {
    return false;
  }
  advance_cursor();
  return true;
}

auto Tokenizer::peek() noexcept -> char
{
  if (is_at_end()) {
    return '\0';
  }
  return source_.at(current_cursor_);
}

auto Tokenizer::peek_next() const noexcept -> char
{
  if (current_cursor_ + 1 >= source_.size()) {
    return '\0';  // LCOV_EXCL_LINE
  }
  return source_.at(current_cursor_ + 1);
}

auto Tokenizer::add_string_token() -> std::optional<SyntaxError>
{
  while (peek() != '"' and !is_at_end()) {
    if (peek() == '\n') {
      handle_newline();
    }
    advance();
  }
  if (is_at_end()) {
    return create_error(SyntaxErrorKind::NonTerminatedStringError);
  } else {
    advance();  // consume last '"'
  }
  add_token(TokenType::String);
  return std::nullopt;
}

auto Tokenizer::add_number_token() -> std::optional<SyntaxError>
{
  auto is_convertible = [](const std::string_view & str) {
    try {
      [[maybe_unused]] const double d = boost::lexical_cast<double>(str);
      return true;
    } catch (...) {
      return false;
    }
  };

  while ((is_digit(peek()) or is_alpha(peek())) and !is_at_end()) {
    advance();
  }
  if (is_at_end()) {
    if (is_convertible(std::string_view(source_).substr(
          current_ctx_start_cursor_, current_cursor_ - current_ctx_start_cursor_))) {
      add_token(TokenType::Number);
      return std::nullopt;
    } else {
      return create_error(SyntaxErrorKind::InvalidNumberError);
    }
  }
  if (peek() == '.') {
    if (!is_digit(peek_next())) {
      return create_error(SyntaxErrorKind::InvalidNumberError);
    }
    advance();  // consume '.'
    while ((is_digit(peek()) or is_alpha(peek())) and !is_at_end()) {
      advance();
    }
    if (is_at_end()) {
      if (is_convertible(std::string_view(source_).substr(
            current_ctx_start_cursor_, current_cursor_ - current_ctx_start_cursor_))) {
        add_token(TokenType::Number);
        return std::nullopt;
      } else {
        return create_error(SyntaxErrorKind::InvalidNumberError);
      }
    }
  }

  if (is_convertible(std::string_view(source_).substr(
        current_ctx_start_cursor_, current_cursor_ - current_ctx_start_cursor_))) {
    add_token(TokenType::Number);
    return std::nullopt;
  } else {
    return create_error(SyntaxErrorKind::InvalidNumberError);
  }
}

auto Tokenizer::add_identifier_token() -> std::optional<SyntaxError>
{
  auto is_alpha_or_digit = [](const auto c) { return is_alpha(c) or is_digit(c); };
  while (is_alpha_or_digit(peek()) and !is_at_end()) {
    advance();
  }
  if (is_at_end()) {
    add_token(TokenType::Identifier);
    return std::nullopt;
  }
  add_token(TokenType::Identifier);
  return std::nullopt;
}

auto Tokenizer::handle_newline() -> void
{
  line_++;
  current_line_start_index_ = current_cursor_;
  const auto line_end = get_next_newline_or_eof();
  if (current_line_start_index_ < line_end) {
    lines_.emplace_back(
      std::make_shared<Line>(std::nullopt, line_, current_line_start_index_, line_end));
  }
}

auto Tokenizer::advance_cursor() -> void
{
  current_cursor_++;
}

auto Tokenizer::get_next_newline_or_eof() const noexcept -> size_t
{
  for (size_t i = current_cursor_; i < source_.size(); ++i) {
    const auto c = source_.at(i);
    if (c == '\n' || c == '\0' || c == std::char_traits<char>::eof()) {
      return i;
    }
  }
  return source_.size() - 1;
}

auto Tokenizer::create_error(const SyntaxErrorKind kind) -> SyntaxError
{
  return SyntaxError{kind, lines_.back(), current_ctx_start_cursor_, current_cursor_};
}  // LCOV_EXCL_LINE

}  // namespace tokenizer
}  // namespace lox
