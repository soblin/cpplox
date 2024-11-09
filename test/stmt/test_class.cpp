#include <cpplox/debug.hpp>
#include <cpplox/interpreter.hpp>
#include <cpplox/parser.hpp>
#include <cpplox/tokenizer.hpp>

#include <gtest/gtest.h>

TEST(Class, check_parse)
{
  const std::string source1 = R"(
class Foo {
  fun bar(a) {
    print a;
    return;
  }
}

var foo = Foo();
)";
  auto tokenizer = lox::Tokenizer(source1);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
  const auto & tokens = lox::as_variant<lox::Tokens>(result);

  auto parser = lox::Parser(tokens);
  const auto parse_result = parser.program();
  EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
  const auto & program = lox::as_variant<lox::Program>(parse_result);

  lox::Interpreter interpreter{};
  const auto err_opt = interpreter.execute(program);
  EXPECT_EQ(err_opt.has_value(), false);
}

TEST(Class, check_field)
{
  const std::string source1 = R"(
var a = 100;
class Foo {
  fun bar(a) {
    print a;
    return;
  }
}

var foo = Foo();
foo.a = "Hello World!";
a = foo.a;
)";
  auto tokenizer = lox::Tokenizer(source1);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
  const auto & tokens = lox::as_variant<lox::Tokens>(result);

  auto parser = lox::Parser(tokens);
  const auto parse_result = parser.program();
  EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
  const auto & program = lox::as_variant<lox::Program>(parse_result);

  lox::Interpreter interpreter{};
  const auto err_opt = interpreter.execute(program);
  EXPECT_EQ(err_opt.has_value(), false);

  const auto a_opt = interpreter.get_variable(tokens[1]);
  EXPECT_EQ(a_opt.has_value(), true);
  EXPECT_EQ(lox::is_variant_v<std::string>(a_opt.value()), true);
  EXPECT_EQ(lox::as_variant<std::string>(a_opt.value()), "Hello World!");
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
