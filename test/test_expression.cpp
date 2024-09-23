#include <cpplox/parser.hpp>
#include <cpplox/tokenizer.hpp>
#include <cpplox/variant.hpp>

#include <gtest/gtest.h>

TEST(Evaluate, math)
{
  const std::string source = R"(-123 * ((45.67) / (89.0)) - 1.23 + (4.567))";
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
  const auto & tokens = lox::as_variant<lox::Tokens>(result);

  auto parser = lox::Parser(tokens);
  const auto parse_result = parser.expression();
  EXPECT_EQ(lox::is_variant_v<lox::Expr>(parse_result), true);

  const auto & expr = lox::as_variant<lox::Expr>(parse_result);
  const auto & eval_opt = lox::evaluate_expr(expr);
  EXPECT_EQ(lox::is_variant_v<lox::Value>(eval_opt), true);
  const auto & eval = lox::as_variant<lox::Value>(eval_opt);
  EXPECT_EQ(lox::is_variant_v<double>(eval), true);
  EXPECT_FLOAT_EQ(lox::as_variant<double>(eval), -123 * ((45.67) / (89.0)) - 1.23 + (4.567));
}

TEST(Evaluate, compare)
{
  {
    const std::string source = R"(-123 * ((45.67) / (89.0)) > -124 * ((45.67) / (89.0)))";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.expression();
    EXPECT_EQ(lox::is_variant_v<lox::Expr>(parse_result), true);

    const auto & expr = lox::as_variant<lox::Expr>(parse_result);
    const auto & eval_opt = lox::evaluate_expr(expr);
    EXPECT_EQ(lox::is_variant_v<lox::Value>(eval_opt), true);
    const auto & eval = lox::as_variant<lox::Value>(eval_opt);
    EXPECT_EQ(lox::is_variant_v<bool>(eval), true);
    EXPECT_EQ(lox::as_variant<bool>(eval), -123 * ((45.67) / (89.0)) > -124 * ((45.67) / (89.0)));
  }
  {
    const std::string source = R"(-123 * ((45.67) / (89.0)) >= -124 * ((45.67) / (89.0)))";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.expression();
    EXPECT_EQ(lox::is_variant_v<lox::Expr>(parse_result), true);

    const auto & expr = lox::as_variant<lox::Expr>(parse_result);
    const auto & eval_opt = lox::evaluate_expr(expr);
    EXPECT_EQ(lox::is_variant_v<lox::Value>(eval_opt), true);
    const auto & eval = lox::as_variant<lox::Value>(eval_opt);
    EXPECT_EQ(lox::is_variant_v<bool>(eval), true);
    EXPECT_EQ(lox::as_variant<bool>(eval), -123 * ((45.67) / (89.0)) > -124 * ((45.67) / (89.0)));
  }

  {
    const std::string source = R"(-123 * ((45.67) / (89.0)) < -122 * ((45.67) / (89.0)))";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.expression();
    EXPECT_EQ(lox::is_variant_v<lox::Expr>(parse_result), true);

    const auto & expr = lox::as_variant<lox::Expr>(parse_result);
    const auto & eval_opt = lox::evaluate_expr(expr);
    EXPECT_EQ(lox::is_variant_v<lox::Value>(eval_opt), true);
    const auto & eval = lox::as_variant<lox::Value>(eval_opt);
    EXPECT_EQ(lox::is_variant_v<bool>(eval), true);
    EXPECT_EQ(lox::as_variant<bool>(eval), -123 * ((45.67) / (89.0)) > -124 * ((45.67) / (89.0)));
  }
  {
    const std::string source = R"(-123 * ((45.67) / (89.0)) <= -122 * ((45.67) / (89.0)))";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.expression();
    EXPECT_EQ(lox::is_variant_v<lox::Expr>(parse_result), true);

    const auto & expr = lox::as_variant<lox::Expr>(parse_result);
    const auto & eval_opt = lox::evaluate_expr(expr);
    EXPECT_EQ(lox::is_variant_v<lox::Value>(eval_opt), true);
    const auto & eval = lox::as_variant<lox::Value>(eval_opt);
    EXPECT_EQ(lox::is_variant_v<bool>(eval), true);
    EXPECT_EQ(lox::as_variant<bool>(eval), -123 * ((45.67) / (89.0)) > -124 * ((45.67) / (89.0)));
  }
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
