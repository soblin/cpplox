#include <cpplox/debug.hpp>
#include <cpplox/parser.hpp>
#include <cpplox/tokenizer.hpp>
#include <cpplox/variant.hpp>

#include <gtest/gtest.h>

void CheckParseTokensTest(const std::string & source)
{
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
}

lox::Tokens ParseTokensTest(const std::string & source)
{
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  const auto & tokens = lox::as_variant<lox::Tokens>(result);
  return tokens;
}

TEST(Tokenizer, parse_success_simple_expr)
{
  const std::string source = R"(-123 * (45.67))";
  ASSERT_NO_FATAL_FAILURE(CheckParseTokensTest(source));
  const auto & tokens = ParseTokensTest(source);
  EXPECT_EQ(tokens.size(), 6);

  auto parser = lox::Parser(tokens);
  const auto parse_result = parser.expression();
  EXPECT_EQ(lox::is_variant_v<lox::Expr>(parse_result), true);

  const auto & expr = lox::as_variant<lox::Expr>(parse_result);
  EXPECT_EQ(lox::to_lisp_repr(expr), "(* (- 123) (group 45.67))");
}

using TestSyntaxErrorKindParamT = std::pair<const std::string, lox::SyntaxErrorKind>;

class TestSyntaxErrorKindTokenizer : public ::testing::TestWithParam<TestSyntaxErrorKindParamT>
{
};

TEST_P(TestSyntaxErrorKindTokenizer, tokenizer_syntax_errors)
{
  const auto [source, kind] = GetParam();
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(result), true);
  const auto & err = lox::as_variant<lox::SyntaxError>(result);

  EXPECT_EQ(err.kind, kind);
}

INSTANTIATE_TEST_SUITE_P(
  TestSyntaxErrorKindTokenizer, TestSyntaxErrorKindTokenizer,
  ::testing::Values(
    //
    TestSyntaxErrorKindParamT{
      R"("abc" + "def" + "ghi)", lox::SyntaxErrorKind::NonTerminatedStringError},
    //
    TestSyntaxErrorKindParamT{R"(_this@ = 123)", lox::SyntaxErrorKind::InvalidCharacterError},
    //
    TestSyntaxErrorKindParamT{
      R"("abc" + "def" + "gh)", lox::SyntaxErrorKind::NonTerminatedStringError}));

TEST(Tokenizer, parse_errors)
{
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
    EXPECT_EQ(err.line->number, 1);
    EXPECT_EQ(err.get_lexical_column(), 8);
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
