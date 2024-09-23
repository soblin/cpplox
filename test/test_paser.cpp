#include <cpplox/parser.hpp>
#include <cpplox/tokenizer.hpp>
#include <cpplox/variant.hpp>

#include <gtest/gtest.h>

TEST(Tokenizer, parse_success_simple_expr)
{
  const std::string source = R"(-123 * (45.67))";
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);

  const auto & tokens = lox::as_variant<lox::Tokens>(result);
  EXPECT_EQ(tokens.size(), 6);

  auto parser = lox::Parser(tokens);
  const auto parse_result = parser.expression();
  EXPECT_EQ(parse_result.has_value(), true);

  EXPECT_EQ(lox::to_lisp_repr(parse_result.value()), "(* (- 123) (group 45.67))");
}

TEST(Tokenizer, parse_fail_simple_expr)
{
  const std::string source = R"(-123 * (45.67 + 123)";
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);

  const auto & tokens = lox::as_variant<lox::Tokens>(result);

  auto parser = lox::Parser(tokens);
  const auto parse_result = parser.expression();

  EXPECT_EQ(parse_result.has_value(), false);
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
