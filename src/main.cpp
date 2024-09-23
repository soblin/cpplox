#include <cpplox/error.hpp>
#include <cpplox/expression.hpp>
#include <cpplox/parser.hpp>
#include <cpplox/variant.hpp>

#include <readline/history.h>
#include <readline/readline.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include <magic_enum.hpp>

enum class EvalKind {
  Ok,
  Error,
};

class Readline
{
public:
  explicit Readline(const std::string & prompt)
  {
    input_ = readline(prompt.c_str());
    if (input_ != nullptr) {
      add_history(input_);
    }
  }
  ~Readline()
  {
    if (input_) {
      free(input_);
    }
  }

  std::optional<std::string_view> get_input() const
  {
    if (input_) {
      return std::make_optional<std::string_view>(input_);
    } else {
      return std::nullopt;
    }
  }

private:
  char * input_{nullptr};
};
auto report(const int line, const std::string & where, const std::string & message) -> void
{
  std::cout << "[line " << line << "] Error" << where << ": " << message << std::endl;
}

auto error(const int line, const std::string & message) -> void
{
  report(line, "", message);
}

auto run(const std::string_view str) -> EvalKind
{
  std::cout << str << std::endl;
  return EvalKind::Ok;
}

auto runFile(const char * path) -> EvalKind
{
  std::ifstream ifs(path);
  if (!ifs) {
    std::cout << path << " does not exist" << std::endl;
    return EvalKind::Error;
  }
  std::stringstream ss;
  ss << ifs.rdbuf();
  return run(ss.str());
}

auto runPrompt() -> void
{
  for (;;) {
    const auto reader = Readline(">>> ");
    if (const auto prompt_opt = reader.get_input(); prompt_opt) {
      [[maybe_unused]] const auto eval = run(prompt_opt.value());
    } else {
      return;
    }
  }
}

auto REPL(int argc, char ** argv) -> int
{
  if (argc > 2) {
    std::cout << "usage: jlox <script>" << std::endl;
    return 0;
  }
  if (argc == 2) {
    runFile(argv[1]);
  } else {
    runPrompt();
  }
  return 0;
}

auto main() -> int
{
  const lox::Expr expr = lox::Binary{
    lox::Unary{
      lox::Token{lox::TokenType::Minus, "-", 1, 0},
      lox::Literal{lox::TokenType::Number, "123", 1, 0}},            // -123
    lox::Token{lox::TokenType::Star, "*", 1, 0},                     // *
    lox::Group{lox::Literal{lox::TokenType::Number, "45.67", 1, 0}}  // (45.67)
  };
  std::cout << lox::to_lisp_repr(expr) << std::endl;

  using lox::Token;
  using lox::Tokens;
  {
    Tokens tokens{
      Token{lox::TokenType::Minus, "-", 1, 0},      Token{lox::TokenType::Number, "123", 1, 0},
      Token{lox::TokenType::Star, "*", 1, 0},       Token{lox::TokenType::LeftParen, "(", 1, 0},
      Token{lox::TokenType::Number, "45.67", 1, 0}, Token{lox::TokenType::RightParen, ")", 1, 0}};
    auto parser = lox::Parser(tokens);
    auto parsed = parser.expression();
    if (lox::is_variant_v<lox::Expr>(parsed)) {
      std::cout << "parse success" << std::endl;
    }
  }

  {
    Tokens tokens{
      Token{lox::TokenType::Minus, "-", 1, 0}, Token{lox::TokenType::Number, "123", 1, 0},
      Token{lox::TokenType::Star, "*", 1, 0}, Token{lox::TokenType::LeftParen, "(", 1, 0},
      Token{lox::TokenType::Number, "45.67", 1, 0}};
    auto parser = lox::Parser(tokens);
    auto parsed = parser.expression();
    if (lox::is_variant_v<lox::Expr>(parsed)) {
      std::cout << "parse success" << std::endl;
    }
  }
  std::cout << magic_enum::enum_name<lox::ParseErrorKind::InvalidLiteralError>() << std::endl;
  return 0;
}
