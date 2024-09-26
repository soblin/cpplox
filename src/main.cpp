#include <cpplox/error.hpp>
#include <cpplox/expression.hpp>
#include <cpplox/parser.hpp>
#include <cpplox/tokenizer.hpp>
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

auto run(const std::string_view str) -> std::optional<lox::SyntaxError>
{
  auto tokenizer = lox::Tokenizer(std::string(str));
  const auto result = tokenizer.take_tokens();
  if (lox::is_variant_v<lox::SyntaxError>(result)) {
    return lox::as_variant<lox::SyntaxError>(result);
  }
  auto parser = lox::Parser(lox::as_variant<lox::Tokens>(result));
  const auto program_result = parser.program();
  if (lox::is_variant_v<lox::SyntaxError>(program_result)) {
    return lox::as_variant<lox::SyntaxError>(program_result);
  } else {
    return std::nullopt;
  }
}

auto runFile(const char * path) -> int
{
  std::ifstream ifs(path);
  if (!ifs) {
    std::cerr << path << " does not exist" << std::endl;
    return 1;
  }
  std::stringstream ss;
  ss << ifs.rdbuf();
  const auto exec = run(ss.str());
  if (exec) {
    std::cerr << magic_enum::enum_name(exec.value().kind) << " at line " << exec.value().line
              << ", column " << exec.value().column << std::endl;
    return 1;
  }
  return 0;
}

auto runPrompt() -> int
{
  for (;;) {
    const auto reader = Readline(">>> ");
    if (const auto prompt_opt = reader.get_input(); prompt_opt) {
      const auto exec = run(prompt_opt.value());
      if (exec) {
        std::cerr << magic_enum::enum_name(exec.value().kind) << " at line " << exec.value().line
                  << ", column " << exec.value().column << std::endl;
        return 1;
      }
    } else {
      return 1;
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
    return runFile(argv[1]);
  } else {
    return runPrompt();
  }
}

auto main(int argc, char ** argv) -> int
{
  return REPL(argc, argv);
}
