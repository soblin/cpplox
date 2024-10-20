#include <cpplox/error.hpp>
#include <cpplox/expression.hpp>
#include <cpplox/interpreter.hpp>
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

  std::optional<std::string> get_input() const
  {
    if (input_) {
      return std::make_optional<std::string>(input_);
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

auto run(lox::Interpreter & interpreter, const std::string & program)
  -> std::variant<std::monostate, lox::SyntaxError, lox::RuntimeError>
{
  auto tokenizer = lox::Tokenizer(program);
  const auto result = tokenizer.take_tokens();
  if (lox::is_variant_v<lox::SyntaxError>(result)) {
    return lox::as_variant<lox::SyntaxError>(result);
  }
  const auto & tokens = lox::as_variant<lox::Tokens>(result);
  auto parser = lox::Parser(tokens);
  const auto program_result = parser.program();
  if (lox::is_variant_v<lox::SyntaxError>(program_result)) {
    return lox::as_variant<lox::SyntaxError>(program_result);
  }
  const auto exec_opt =
    interpreter.execute(lox::as_variant<std::vector<lox::Declaration>>(program_result));
  if (exec_opt) {
    return exec_opt.value();
  }
  return std::monostate{};
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
  lox::Interpreter interpreter;
  const auto exec_opt = run(interpreter, ss.str());
  if (lox::is_variant_v<lox::SyntaxError>(exec_opt)) {
    const auto & err = lox::as_variant<lox::SyntaxError>(exec_opt);
    std::cout << err.get_line_string(2);
    std::cout << err.get_visualization_string(ss.str(), 4);
    return 1;
  }
  if (lox::is_variant_v<lox::RuntimeError>(exec_opt)) {
    const auto & exec = lox::as_variant<lox::RuntimeError>(exec_opt);
    std::cout << lox::get_line_string(exec, 2);
    std::cout << lox::get_visualization_string(ss.str(), exec, 4);
    return 1;
  }
  return 0;
}

auto runPrompt() -> int
{
  // NOTE: if this interpreter is instantiated in each cycle, it will "forget" the prior
  // environment, thus we cannot reuse the variables we have defined in the previous prompt
  lox::Interpreter interpreter;
  std::vector<std::string> prompts;

  for (;;) {
    const auto reader = Readline(">>> ");
    if (const auto prompt_opt = reader.get_input(); prompt_opt) {
      prompts.push_back(prompt_opt.value());
      const auto & prompt = prompts.back();
      const auto exec_opt = run(interpreter, prompt);
      if (lox::is_variant_v<lox::SyntaxError>(exec_opt)) {
        const auto & exec = lox::as_variant<lox::SyntaxError>(exec_opt);
        std::cout << exec.get_line_string(2);
        std::cout << exec.get_visualization_string(prompt, 4);
      }
      if (lox::is_variant_v<lox::RuntimeError>(exec_opt)) {
        const auto & exec = lox::as_variant<lox::RuntimeError>(exec_opt);
        std::cout << lox::get_line_string(exec, 2);
        std::cout << lox::get_visualization_string(prompt, exec, 4);
      }
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
