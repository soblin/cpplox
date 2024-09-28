#include <cpplox/debug.hpp>
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
  EXPECT_EQ(lox::is_variant_v<lox::Expr>(parse_result), true);

  const auto & expr = lox::as_variant<lox::Expr>(parse_result);
  EXPECT_EQ(lox::to_lisp_repr(expr), "(* (- 123) (group 45.67))");
}

TEST(Tokenizer, parse_errors)
{
  {
    const std::string source = R"("abc" + "def" + "ghi)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(result);

    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::NonTerminatedStringError);
  }

  {
    const std::string source = R"(-123 * (45.67 + 123)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);

    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.expression();

    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::UnmatchedParenError);
    EXPECT_EQ(err.line, 1);
    EXPECT_EQ(err.column, 8);
  }

  {
    const std::string source = R"(_this@ = 123)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(result), true);

    const auto & err = lox::as_variant<lox::SyntaxError>(result);

    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::InvalidCharacterError);
  }

  {
    const std::string source = R"("abc" + "def" + "gh)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(result), true);

    const auto & err = lox::as_variant<lox::SyntaxError>(result);

    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::NonTerminatedStringError);
  }

  {
    const std::string source = R"(123 * (123 * 456 + 789 -))";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);

    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.expression();

    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::InvalidLiteralError);
  }
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
