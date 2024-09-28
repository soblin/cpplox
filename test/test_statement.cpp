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
    const std::string source1 = R"(
var a = (1 + 2) * (3 + 45.6);
var b = a;
var c = a * b;
var d = 123.456;
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
    const auto c = lox::as_variant<double>(c_opt.value());
    EXPECT_FLOAT_EQ(c, a * b);

    auto d_opt = interpreter.get_variable(tokens[28]);
    EXPECT_EQ(d_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<double>(d_opt.value()), true);
    EXPECT_FLOAT_EQ(lox::as_variant<double>(d_opt.value()), 123.456);

    const std::string source2 = R"(d = c;)";
    const auto program2 = lox::as_variant<lox::Program>(
      lox::Parser(lox::as_variant<lox::Tokens>(lox::Tokenizer(source2).take_tokens())).program());
    interpreter.execute(program2);
    d_opt = interpreter.get_variable(tokens[28]);
    EXPECT_EQ(d_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<double>(d_opt.value()), true);
    EXPECT_FLOAT_EQ(lox::as_variant<double>(d_opt.value()), c);
  }
  {
    const std::string source = R"(
var a;
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
    const auto a = lox::as_variant<lox::Nil>(a_opt.value());
    EXPECT_EQ(a, lox::Nil{});
  }
}

TEST(Statement, expr_statement_errors)
{
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
(1 + 2) * ( 3 + "str");
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
    const auto exec = interpreter.execute(program);
    EXPECT_EQ(exec.has_value(), true);
    EXPECT_EQ(exec.value().kind, lox::RuntimeErrorKind::TypeError);
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
print (1 + 2) * ( 3 + "str");
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
    const auto exec = interpreter.execute(program);
    EXPECT_EQ(exec.has_value(), true);
    EXPECT_EQ(exec.value().kind, lox::RuntimeErrorKind::TypeError);
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
  {
    const std::string source = R"(
var a = (1 + 2) * (3 + 45.6);
var b = a;
var c = a * d;
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
    const auto exec = interpreter.execute(program);
    EXPECT_EQ(exec.has_value(), true);
    EXPECT_EQ(exec.value().kind, lox::RuntimeErrorKind::UndefinedVariable);
  }

  {
    const std::string source = R"(
var a = (1 + 2) * (3 + "str");
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
    const auto exec = interpreter.execute(program);
    EXPECT_EQ(exec.has_value(), true);
    EXPECT_EQ(exec.value().kind, lox::RuntimeErrorKind::TypeError);
  }

  {
    const std::string source = R"(
(b) = 3;
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::InvalidAssignmentTarget);
  }

  {
    const std::string source = R"(
var a;
a = (1 + 2) * (3 + "str");
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
    const auto exec = interpreter.execute(program);
    EXPECT_EQ(exec.has_value(), true);
    EXPECT_EQ(exec.value().kind, lox::RuntimeErrorKind::TypeError);
  }
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
