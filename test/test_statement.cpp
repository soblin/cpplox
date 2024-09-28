#include "cpplox/error.hpp"
#include "cpplox/statement.hpp"
#include <cpplox/interpreter.hpp>
#include <cpplox/parser.hpp>
#include <cpplox/tokenizer.hpp>
#include <cpplox/variant.hpp>

#include <gtest/gtest.h>

TEST(Statement, expr_statement)
{
  {
    const std::string source = R"(
(1 + 2) * ( 3 + 4);
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);
    EXPECT_EQ(lox::is_variant_v<lox::ExprStmt>(program[0]), true);
    const auto & stmt1 = lox::as_variant<lox::ExprStmt>(program[0]);

    auto interpreter = lox::Interpreter{};
    const auto & eval_opt = interpreter.evaluate_expr(stmt1.expression);
    EXPECT_EQ(lox::is_variant_v<lox::Value>(eval_opt), true);
    const auto & eval = lox::as_variant<lox::Value>(eval_opt);
    EXPECT_EQ(lox::is_variant_v<int64_t>(eval), true);
    EXPECT_EQ(lox::as_variant<int64_t>(eval), (1 + 2) * (3 + 4));
  }
  {
    const std::string source = R"(
(1 + 2) * ( 3 + 4)
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::StmtWithoutSemicolun);
  }

  {
    const std::string source = R"(
print (1 + 2) * ( 3 + 4);
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);
    EXPECT_EQ(lox::is_variant_v<lox::PrintStmt>(program[0]), true);
    const auto & stmt1 = lox::as_variant<lox::PrintStmt>(program[0]);

    auto interpreter = lox::Interpreter{};
    const auto & eval_opt = interpreter.evaluate_expr(stmt1.expression);
    EXPECT_EQ(lox::is_variant_v<lox::Value>(eval_opt), true);
    const auto & eval = lox::as_variant<lox::Value>(eval_opt);
    EXPECT_EQ(lox::is_variant_v<int64_t>(eval), true);
    EXPECT_EQ(lox::as_variant<int64_t>(eval), (1 + 2) * (3 + 4));
  }
  {
    const std::string source = R"(
print (1 + 2) * ( 3 + 4)
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::StmtWithoutSemicolun);
  }

  {
    const std::string source = R"(
var a = (1 + 2) * ( 3 + 4);
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    EXPECT_EQ(tokens.size(), 15);
    EXPECT_EQ(tokens[14].type, lox::TokenType::Semicolun);
    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);
    EXPECT_EQ(lox::is_variant_v<lox::VarDeclStmt>(program[0]), true);
    const auto & stmt1 = lox::as_variant<lox::VarDeclStmt>(program[0]);

    auto interpreter = lox::Interpreter{};
    const auto & eval_opt = interpreter.evaluate_expr(stmt1.initializer.value());
    EXPECT_EQ(lox::is_variant_v<lox::Value>(eval_opt), true);
    const auto & eval = lox::as_variant<lox::Value>(eval_opt);
    EXPECT_EQ(lox::is_variant_v<int64_t>(eval), true);
    EXPECT_EQ(lox::as_variant<int64_t>(eval), (1 + 2) * (3 + 4));
  }
  {
    const std::string source = R"(
foo = (1 + 2) * ( 3 + 4)
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.var_decl();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::StmtWithoutSemicolun);
  }
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
