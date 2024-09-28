#include <cpplox/error.hpp>
#include <cpplox/interpreter.hpp>
#include <cpplox/parser.hpp>
#include <cpplox/statement.hpp>
#include <cpplox/tokenizer.hpp>
#include <cpplox/variant.hpp>

#include <gtest/gtest.h>

TEST(debug, debug)
{
  {
    const std::string source = R"(
var a = (1 + 2) * (3 + 45.6);
var b = a;
var c = a * b;
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    lox::Interpreter interpreter{};
    interpreter.execute(program);
    const auto a_opt = interpreter.get_variable(tokens[1]);
    EXPECT_EQ(a_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<double>(a_opt.value()), true);
    const auto a = lox::as_variant<double>(a_opt.value());
    EXPECT_FLOAT_EQ(a, (1 + 2) * (3 + 45.6));

    const auto b_opt = interpreter.get_variable(tokens[16]);
    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<double>(b_opt.value()), true);
    const auto b = lox::as_variant<double>(b_opt.value());
    EXPECT_FLOAT_EQ(b, (1 + 2) * (3 + 45.6));

    const auto c_opt = interpreter.get_variable(tokens[21]);
    EXPECT_EQ(c_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<double>(c_opt.value()), true);
    EXPECT_FLOAT_EQ(lox::as_variant<double>(c_opt.value()), a * b);
  }
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
