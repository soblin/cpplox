#include <cpplox/debug.hpp>
#include <cpplox/error.hpp>
#include <cpplox/interpreter.hpp>
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

void CheckParseProgramTest(const std::string & source)
{
  ASSERT_NO_FATAL_FAILURE(CheckParseTokensTest(source));
  const auto & tokens = ParseTokensTest(source);
  auto parser = lox::Parser(tokens);
  const auto parse_result = parser.program();
  EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
}

std::pair<lox::Program, lox::Tokens> ParseProgramTest(const std::string & source)
{
  const auto & tokens = ParseTokensTest(source);
  auto parser = lox::Parser(tokens);
  const auto parse_result = parser.program();
  return {lox::as_variant<lox::Program>(parse_result), tokens};
}

using TestExprSyntaxErrorParam = std::tuple<const char *, lox::SyntaxErrorKind>;

class TestExprSyntaxError : public ::testing::TestWithParam<TestExprSyntaxErrorParam>
{
};

TEST_P(TestExprSyntaxError, expr_syntax_error)
{
  const auto [source_str, kind] = GetParam();
  const auto source = std::string(source_str);
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
  const auto & tokens = lox::as_variant<lox::Tokens>(result);

  auto parser = lox::Parser(tokens);
  const auto parse_result = parser.expression();
  EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);

  const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
  EXPECT_EQ(err.kind, kind);
}

INSTANTIATE_TEST_SUITE_P(
  TestExprSyntaxError, TestExprSyntaxError,
  ::testing::Values(
    //
    TestExprSyntaxErrorParam{R"(123 == (1+2)", lox::SyntaxErrorKind::UnmatchedParenError},
    //
    TestExprSyntaxErrorParam{R"(123 < (1+2)", lox::SyntaxErrorKind::UnmatchedParenError},
    //
    TestExprSyntaxErrorParam{R"(-(1+2)", lox::SyntaxErrorKind::UnmatchedParenError}));

class TestExprRuntimeError : public ::testing::TestWithParam<const char *>
{
};

TEST_P(TestExprRuntimeError, expr_runtime_error)
{
  const auto source_str = GetParam();
  const auto source = std::string(source_str);
  auto tokenizer = lox::Tokenizer(source);
  const auto result = tokenizer.take_tokens();
  EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
  const auto & tokens = lox::as_variant<lox::Tokens>(result);

  auto parser = lox::Parser(tokens);
  const auto parse_result = parser.expression();
  EXPECT_EQ(lox::is_variant_v<lox::Expr>(parse_result), true);

  auto interpreter = lox::Interpreter{};
  const auto & expr = lox::as_variant<lox::Expr>(parse_result);
  const auto & eval_opt = interpreter.evaluate_expr(expr);
  EXPECT_EQ(lox::is_variant_v<lox::RuntimeError>(eval_opt), true);
}

INSTANTIATE_TEST_SUITE_P(
  TestExprRuntimeError, TestExprRuntimeError,
  ::testing::Values(
    R"(123 + "abc")", R"("abc" - 123)", R"(123 * "abc")", R"(123 / "abc")", R"(-"abc")",
    R"("abc" < 123)", R"("abc" <= 123)", R"("abc" > 123)", R"("abc" >= 123)",
    R"(123 + 456 * "789")", R"(456 * "789" + 123)", R"(123 == 456 * "123")", R"(123 > 456 * "123")",
    R"(-(123 * "456"))"));

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
