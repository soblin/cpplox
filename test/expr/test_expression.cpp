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

template <typename T>
void test_as_t(const std::optional<lox::Value> & value_opt, const T & test)
{
  EXPECT_EQ(value_opt.has_value(), true);
  EXPECT_EQ(lox::is_variant_v<T>(value_opt.value()), true);
  EXPECT_EQ(lox::as_variant<T>(value_opt.value()), test);
}

void test_as(
  const std::optional<lox::Value> & value_opt, const lox::Value & test, const size_t type_index)
{
  if (type_index == lox::helper::Nil_Index) {
    ASSERT_NO_FATAL_FAILURE(test_as_t<lox::Nil>(value_opt, lox::as_variant<lox::Nil>(test)));
  } else if (type_index == lox::helper::bool_Index) {
    ASSERT_NO_FATAL_FAILURE(test_as_t<bool>(value_opt, lox::as_variant<bool>(test)));
  } else if (type_index == lox::helper::long_Index) {
    ASSERT_NO_FATAL_FAILURE(test_as_t<int64_t>(value_opt, lox::as_variant<int64_t>(test)));
  } else if (type_index == lox::helper::double_Index) {
    ASSERT_NO_FATAL_FAILURE(test_as_t<double>(value_opt, lox::as_variant<double>(test)));
  } else if (type_index == lox::helper::str_Index) {
    ASSERT_NO_FATAL_FAILURE(test_as_t<std::string>(value_opt, lox::as_variant<std::string>(test)));
  }
}

using TestExprValueParam = std::tuple<const char *, size_t, lox::Value>;

class TestExprValue : public ::testing::TestWithParam<TestExprValueParam>
{
};

TEST_P(TestExprValue, expr_value)
{
  const auto [source_str, type_index, test] = GetParam();
  const auto source = std::string(source_str);
  ASSERT_NO_FATAL_FAILURE(CheckParseTokensTest(source));
  const auto tokens = ParseTokensTest(source);

  auto parser = lox::Parser(tokens);
  const auto parse_result = parser.expression();
  EXPECT_EQ(lox::is_variant_v<lox::Expr>(parse_result), true);

  auto interpreter = lox::Interpreter{};
  const auto & expr = lox::as_variant<lox::Expr>(parse_result);
  const auto & eval_opt = interpreter.evaluate_expr(expr);
  EXPECT_EQ(lox::is_variant_v<lox::Value>(eval_opt), true);
  const auto & eval = lox::as_variant<lox::Value>(eval_opt);
  ASSERT_NO_FATAL_FAILURE(test_as(eval, test, type_index));
}

INSTANTIATE_TEST_SUITE_P(
  TestExprValues, TestExprValue,
  ::testing::Values(
    /**
     * Math
     */
    //
    TestExprValueParam{R"(45 * 67)", lox::helper::long_Index, int64_t{45 * 67}},
    //
    TestExprValueParam{
      R"(-123 * ((45.67) / (89.0)) - 1.23 + (4.567))", lox::helper::double_Index,
      -123 * ((45.67) / (89.0)) - 1.23 + (4.567)},
    //
    TestExprValueParam{
      R"(-123 * 4.56 + 456.789 * 1234 - 5678.9)", lox::helper::double_Index,
      -123 * 4.56 + 456.789 * 1234 - 5678.9},
    /**
     * Comparison
     */
    //
    TestExprValueParam{R"((123 < 123.456) == (123.45 < 124))", lox::helper::bool_Index, true},
    //
    TestExprValueParam{R"((123 <= 123.456) == (123.45 <= 124))", lox::helper::bool_Index, true},
    //
    TestExprValueParam{R"((123 > 123.456) == (123.45 > 124))", lox::helper::bool_Index, true},
    //
    TestExprValueParam{R"((123 >= 123.456) == (123.45 >= 124))", lox::helper::bool_Index, true},
    //
    TestExprValueParam{R"(123 == 123)", lox::helper::bool_Index, true},
    //
    TestExprValueParam{R"(123.45 == 123.45)", lox::helper::bool_Index, true},
    //
    TestExprValueParam{R"((123 < 124) and ("str" != nil))", lox::helper::bool_Index, true},
    //
    TestExprValueParam{R"((123 < 124) and ("str" == nil))", lox::helper::bool_Index, false},
    //
    TestExprValueParam{R"((123 < 124) or ("str" == nil))", lox::helper::bool_Index, true},
    //
    TestExprValueParam{R"((123 < 124) and "str")", lox::helper::bool_Index, true},
    //
    TestExprValueParam{R"((123 < 124) and nil)", lox::helper::bool_Index, false},
    //
    TestExprValueParam{R"((123 > 124) and nil)", lox::helper::bool_Index, false},
    //
    TestExprValueParam{R"((123 > 124) or nil)", lox::helper::bool_Index, false},
    //
    TestExprValueParam{
      R"(
(-123 * ((45.67) / (89.0))) > (-124 * ((45.67) / (89.0)))
)",
      lox::helper::bool_Index, (-123 * ((45.67) / (89.0))) >= (-124 * ((45.67) / (89.0)))},
    //
    TestExprValueParam{
      R"((-123 * ((45.67) / (89.0))) >= (-124 * ((45.67) / (89.0))))", lox::helper::bool_Index,
      (-123 * ((45.67) / (89.0))) >= (-124 * ((45.67) / (89.0)))},
    //
    TestExprValueParam{
      R"(-123 * ((45.67) / (89.0)) < -122 * ((45.67) / (89.0)))", lox::helper::bool_Index,
      -123 * ((45.67) / (89.0)) > -124 * ((45.67) / (89.0))},
    //
    TestExprValueParam{
      R"(-123.0 * ((45.67) / (89.0)) <= -122 * ((45.67) / (89.0)))", lox::helper::bool_Index,
      -123.0 * ((45.67) / (89.0)) > -124 * ((45.67) / (89.0))},
    //
    TestExprValueParam{R"("abc" == "abc")", lox::helper::bool_Index, true},
    //
    TestExprValueParam{R"("abc" == "abcde")", lox::helper::bool_Index, false},
    //
    TestExprValueParam{R"("abc" == 123)", lox::helper::bool_Index, false},
    //
    TestExprValueParam{R"(nil == nil)", lox::helper::bool_Index, true},
    //
    TestExprValueParam{R"(nil == 123)", lox::helper::bool_Index, false},
    //
    TestExprValueParam{R"("abc" != "abc")", lox::helper::bool_Index, false},
    //
    TestExprValueParam{R"("abc" != "abcde")", lox::helper::bool_Index, true},
    //
    TestExprValueParam{R"("abc" != 123)", lox::helper::bool_Index, true},
    //
    TestExprValueParam{R"(nil != nil)", lox::helper::bool_Index, false},
    //
    TestExprValueParam{R"(nil != 123)", lox::helper::bool_Index, true},
    //
    TestExprValueParam{R"(true == true)", lox::helper::bool_Index, true},
    //
    TestExprValueParam{R"(true == false)", lox::helper::bool_Index, false},
    //
    TestExprValueParam{R"(true == !false)", lox::helper::bool_Index, true},
    //
    TestExprValueParam{R"(true == nil)", lox::helper::bool_Index, false},
    //
    TestExprValueParam{R"(false != nil)", lox::helper::bool_Index, true},
    //
    TestExprValueParam{R"(true == !nil)", lox::helper::bool_Index, true},
    /**
     * String
     */
    TestExprValueParam{
      R"("abc" + "def" + "ghi")", lox::helper::str_Index, std::string("abcdefghi")}));

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
