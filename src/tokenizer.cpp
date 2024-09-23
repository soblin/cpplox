#include <cpplox/tokenizer.hpp>
#include <cpplox/variant.hpp>

#include <boost/lexical_cast.hpp>

namespace lox
{

inline namespace tokenizer
{

Tokenizer::Tokenizer(const std::string & source) : source_(source)
{
}

auto Tokenizer::is_at_end() const noexcept -> bool
{
  return current_ >= source_.size();
}

auto Tokenizer::take_tokens() -> std::variant<Tokens, ParseError>
{
  while (!is_at_end()) {
    start_ = current_;
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
    auto c = source_.at(current_);
    advance_cursor();
    return c;
  } else {
    return std::nullopt;
  }
}

auto Tokenizer::add_token(const TokenType & token_type) -> void
{
  const auto start = (token_type == TokenType::String) ? (start_ + 1) : start_;
  const auto len =
    (token_type == TokenType::String) ? (current_ - start_ - 2) : (current_ - start_);
  const auto text = source_.substr(start, len);
  const auto column = (token_type == TokenType::String) ? column_ - len - 1 : column_ - len + 1;
  if (token_type == TokenType::Identifier and is_keyword(text)) {
    tokens_.emplace_back(keyword_map.find(text)->second, text, line_, column);
    return;
  }
  tokens_.emplace_back(token_type, text, line_, column);
}

auto Tokenizer::scan_new_token() -> std::optional<ParseError>
{
  const auto c_opt = advance();
  if (!c_opt) {
    add_token(TokenType::Eof);
    return std::nullopt;
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
    } else {
      add_token(TokenType::Slash);
    }
    return std::nullopt;
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
  return ParseError{ParseErrorKind::InvalidCharacterError, line_, column_};
}

auto Tokenizer::match(const char expected) noexcept -> bool
{
  if (is_at_end()) {
    return false;
  }
  if (source_.at(current_) != expected) {
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
  return source_.at(current_);
}

auto Tokenizer::add_string_token() -> std::optional<ParseError>
{
  while (peek() != '"' and !is_at_end()) {
    if (peek() == '\n') {
      handle_newline();
    }
    advance();
  }
  if (is_at_end()) {
    const auto str = source_.substr(start_, current_ - start_);
    return std::make_optional<ParseError>(ParseErrorKind::NonTerminatedStringError, line_, column_);
  } else {
    // NOTE: we need to skip the last '"'
    advance();
  }
  add_token(TokenType::String);
  return std::nullopt;
}

auto Tokenizer::add_number_token() -> std::optional<ParseError>
{
  while ((is_digit(peek()) or is_alpha(peek())) and !is_at_end()) {
    advance();
  }

  if (is_at_end()) {
    const auto str = source_.substr(start_, current_ - start_);
    return std::make_optional<ParseError>(ParseErrorKind::NonTerminatedNumberError, line_, column_);
  }

  if (peek() == '.') {
    if (!is_digit(peek_next())) {
      const auto str = source_.substr(start_, current_ - start_ + 2);
      return std::make_optional<ParseError>(ParseErrorKind::InvalidNumberError, line_, column_);
    }
    advance();  // consume '.'
    while ((is_digit(peek()) or is_alpha(peek())) and !is_at_end()) {
      advance();
    }
    if (is_at_end()) {
      const auto str = source_.substr(start_, current_ - start_);
      return std::make_optional<ParseError>(
        ParseErrorKind::NonTerminatedNumberError, line_, column_);
    }
  }

  const auto str = source_.substr(start_, current_ - start_);
  try {
    [[maybe_unused]] const double d = boost::lexical_cast<double>(str);
    add_token(TokenType::Number);
  } catch (...) {
    return std::make_optional<ParseError>(ParseErrorKind::InvalidNumberError, line_, column_);
  }
  return std::nullopt;
}

auto Tokenizer::peek_next() const noexcept -> char
{
  if (current_ + 1 >= source_.size()) {
    return '\0';
  }
  return source_.at(current_ + 1);
}

auto Tokenizer::add_identifier_token() -> std::optional<ParseError>
{
  auto is_alpha_or_digit = [](const auto c) { return is_alpha(c) or is_digit(c); };
  while (is_alpha_or_digit(peek()) and !is_at_end()) {
    advance();
  }
  if (is_at_end()) {
    const auto str = source_.substr(start_, current_ - start_);
    return std::make_optional<ParseError>(ParseErrorKind::InvalidIdentifierError, line_, column_);
  }
  add_token(TokenType::Identifier);
  return std::nullopt;
}

auto Tokenizer::handle_newline() -> void
{
  line_++;
  column_ = 0;
}

auto Tokenizer::advance_cursor() -> void
{
  current_++;
  column_++;
}

}  // namespace tokenizer
}  // namespace lox
