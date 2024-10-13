#include <cpplox/error.hpp>
#include <cpplox/interpreter.hpp>
#include <cpplox/parser.hpp>
#include <cpplox/statement.hpp>
#include <cpplox/tokenizer.hpp>
#include <cpplox/variant.hpp>

#include <boost/variant.hpp>

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
    EXPECT_EQ(lox::is_variant_v<lox::Stmt>(program[0]), true);
    const auto & stmt = lox::as_variant<lox::Stmt>(program[0]);
    EXPECT_EQ(lox::is_variant_v<lox::ExprStmt>(stmt), true);
    const auto & stmt1 = lox::as_variant<lox::ExprStmt>(stmt);

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
    EXPECT_EQ(lox::is_variant_v<lox::Stmt>(program[0]), true);
    const auto & stmt = lox::as_variant<lox::Stmt>(program[0]);
    EXPECT_EQ(lox::is_variant_v<lox::PrintStmt>(stmt), true);
    const auto & stmt1 = lox::as_variant<lox::PrintStmt>(stmt);

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
    EXPECT_EQ(lox::is_variant_v<lox::VarDecl>(program[0]), true);
    const auto & stmt1 = lox::as_variant<lox::VarDecl>(program[0]);

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
    [[maybe_unused]] const auto exec1 = interpreter.execute(program);
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
    [[maybe_unused]] const auto exec2 = interpreter.execute(program2);
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
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto a_opt = interpreter.get_variable(tokens[1]);
    EXPECT_EQ(a_opt.has_value(), true);
    const auto a = lox::as_variant<lox::Nil>(a_opt.value());
    EXPECT_EQ(a, lox::Nil{});
  }
}

TEST(Statement, if_statement)
{
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (a == "123"){
  b = 100;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(b_opt.value()), 100);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (a == "1234"){
  b = 100;
} else {
  b = 1000;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(b_opt.value()), 1000);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (a == "1234"){
  b = 100;
} else if (a == "123") {
  b = 200;
} else {
  b = 300;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(b_opt.value()), 200);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (a == "1234"){
  b = 100;
} else if (a == "123") {
  b = 200;
}

print a;
print b;
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(b_opt.value()), 200);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (a == "1234"){
  b = 100;
} else if (a == "12345") {
  b = 200;
}

print a;
print b;
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(b_opt.value()), 10);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (a == "1234"){
  b = 100;
} else if (a == "12345") {
  b = 200;
} else {
  b = 300;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(b_opt.value()), 300);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (var c = "12345"; c == "12345"){
  b = 100;
} else if (var d = 12345; c == "12345") {
  b = d;
} else {
  b = 300;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(b_opt.value()), 100);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (var c = "12345"; c != "12345"){
  b = 100;
} else if (var d = 12345; c == "12345") {
  b = d;
} else {
  b = 300;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(b_opt.value()), 12345);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (var c = "12345"; c != "12345"){
  b = 100;
} else if (var d = 12345; c != "12345") {
  b = d;
} else if (var e = 123456; d == 12345) {
  b = b + d + e;
} else {
// do nothing
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(b_opt.value()), 10 + 12345 + 123456);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (var c = "12345"; c != "12345"){
  b = 100;
} else if (var d = 12345; c != "12345") {
  b = d;
} else if (var e = 123456; d != 12345) {
  b = b + d + e;
} else {
  b = 54321;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(b_opt.value()), 54321);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
var c = "before";
b = (c == "before") and (c = "after") == "after";
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    const auto c_opt = interpreter.get_variable(tokens[11]);

    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<bool>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<bool>(b_opt.value()), true);

    EXPECT_EQ(c_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<std::string>(c_opt.value()), true);
    EXPECT_EQ(lox::as_variant<std::string>(c_opt.value()), "after");
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
var c = "before";
b = (c == "before") or (c = "after") == "after";
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    const auto c_opt = interpreter.get_variable(tokens[11]);

    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<bool>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<bool>(b_opt.value()), true);

    EXPECT_EQ(c_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<std::string>(c_opt.value()), true);
    EXPECT_EQ(lox::as_variant<std::string>(c_opt.value()), "before");
  }
  //
  {
    const std::string source = R"(
var a = "123";
var b = 10;
var c = "before";
b = (c == "before2") and (c = "after") == "after";
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    const auto c_opt = interpreter.get_variable(tokens[11]);

    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<bool>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<bool>(b_opt.value()), false);

    EXPECT_EQ(c_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<std::string>(c_opt.value()), true);
    EXPECT_EQ(lox::as_variant<std::string>(c_opt.value()), "before");
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
var c = "before";
b = (c == "before2") or (c = "after") == "after";
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    const auto c_opt = interpreter.get_variable(tokens[11]);

    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<bool>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<bool>(b_opt.value()), true);

    EXPECT_EQ(c_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<std::string>(c_opt.value()), true);
    EXPECT_EQ(lox::as_variant<std::string>(c_opt.value()), "after");
  }
  {
    const std::string source = R"(
var a = 123;
var b = 10;
var c = "before";
b = (a == 100 + 23) and (c == "be" + "fore");
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    const auto c_opt = interpreter.get_variable(tokens[11]);

    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<bool>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<bool>(b_opt.value()), true);
  }
  {
    const std::string source = R"(
var a = 0;
var b = 0;
var c = 0;
while (a < 10) {
   c = c + 100;
   while(b < 10) {
      c = c + 10;
      b = b + 1;
   }
   a = a + 1;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto c_opt = interpreter.get_variable(tokens[11]);

    EXPECT_EQ(c_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(c_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(c_opt.value()), 1100);
  }
  {
    const std::string source = R"(
var a = -10;
var b = -10;
var c = 0;
for (a = 0; a < 10; a = a + 1) {
   c = c + 10;
   for (b = 0; b < 10; b = b + 1) {
   }
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto a_opt = interpreter.get_variable(tokens[1]);
    const auto b_opt = interpreter.get_variable(tokens[7]);
    const auto c_opt = interpreter.get_variable(tokens[13]);

    EXPECT_EQ(a_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(a_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(a_opt.value()), 10);

    EXPECT_EQ(a_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(b_opt.value()), 10);

    EXPECT_EQ(c_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(c_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(c_opt.value()), 100);
  }
  {
    const std::string source = R"(
var a = -10;
var b = -10;
var c = 0;
for (a = 0; a < 10; a = a + 1) {
   c = c + 10;
   for (b = 0; b < 10; b = b + 1) {
      c = c + 20;
   }
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto a_opt = interpreter.get_variable(tokens[1]);
    const auto b_opt = interpreter.get_variable(tokens[7]);
    const auto c_opt = interpreter.get_variable(tokens[13]);

    EXPECT_EQ(a_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(a_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(a_opt.value()), 10);

    EXPECT_EQ(a_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(b_opt.value()), 10);

    EXPECT_EQ(c_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(c_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(c_opt.value()), (10 + (10 * 20)) * 10);
  }
  {
    const std::string source = R"(
var a = -10;
var b = -10;
var c = 0;
for (a = 0; a < 10; a = a + 1) {
   c = c + 10;
   for (var d = 0; d < 10; d = d + 1) {
      c = c + 20;
   }
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto a_opt = interpreter.get_variable(tokens[1]);
    const auto c_opt = interpreter.get_variable(tokens[13]);

    EXPECT_EQ(a_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(a_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(a_opt.value()), 10);

    EXPECT_EQ(c_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(c_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(c_opt.value()), (10 + (10 * 20)) * 10);
  }
  {
    const std::string source = R"(
var a = 0;
var b = 0;
var c = 0;
for (; a < 10; a = a + 1) {
   c = c + 10;
   for (; b < 10; b = b + 1) {
      c = c + 20;
   }
   // NOTE: here, already b == 10; from the next iteration of a==1; c+=20 is not executed anymore!
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto a_opt = interpreter.get_variable(tokens[1]);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    const auto c_opt = interpreter.get_variable(tokens[11]);

    EXPECT_EQ(a_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(a_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(a_opt.value()), 10);

    EXPECT_EQ(a_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(b_opt.value()), 10);

    EXPECT_EQ(c_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(c_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(c_opt.value()), 10 * 20 + 10 * 10);
  }
  {
    const std::string source = R"(
var a = 0;
var b = 0;
var c = 0;
for (; a < 10;) {
   c = c + 10;
   for (; b < 10;) {
      c = c + 20;
      b = b + 1;
   }
   // NOTE: here, already b == 10; from the next iteration of a==1; c+=20 is not executed anymore!
   a = a + 1;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto a_opt = interpreter.get_variable(tokens[1]);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    const auto c_opt = interpreter.get_variable(tokens[11]);

    EXPECT_EQ(a_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(a_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(a_opt.value()), 10);

    EXPECT_EQ(a_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(b_opt.value()), 10);

    EXPECT_EQ(c_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(c_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(c_opt.value()), 10 * 20 + 10 * 10);
  }
  {
    const std::string source = R"(
var a = 0;
var b = 0;
for (;;) {
  print a;
  a = a + 1;
  if (a > 10) {
    break;
    b = 10;
  } else {
    continue;
  }
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto a_opt = interpreter.get_variable(tokens[1]);
    const auto b_opt = interpreter.get_variable(tokens[6]);

    EXPECT_EQ(a_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(a_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(a_opt.value()), 11);

    EXPECT_EQ(a_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(b_opt.value()), 0);
  }
  {
    const std::string source = R"(
var a = 0;
var b = 0;
for (;;) {
  a = a + 1;
  if (a > 10) {
    break;
  } else {
    continue;
    b = 10;
  }
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto a_opt = interpreter.get_variable(tokens[1]);
    const auto b_opt = interpreter.get_variable(tokens[6]);

    EXPECT_EQ(a_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(a_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(a_opt.value()), 11);

    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(b_opt.value()), 0);
  }
  {
    const std::string source = R"(
var a = 0;
var b = 0;
var c = 0;
for (;;) {
  a = a + 1;
  if (a > 10) {
    break;
  }
  for (;;) {
    if (b > 10) {
      break;
    }
    b = b + 1;
  }
  c = c + 1;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    [[maybe_unused]] const auto exec = interpreter.execute(program);
    const auto a_opt = interpreter.get_variable(tokens[1]);
    const auto b_opt = interpreter.get_variable(tokens[6]);
    const auto c_opt = interpreter.get_variable(tokens[11]);

    EXPECT_EQ(a_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(a_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(a_opt.value()), 11);

    EXPECT_EQ(b_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(b_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(b_opt.value()), 11);

    EXPECT_EQ(c_opt.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<int64_t>(c_opt.value()), true);
    EXPECT_EQ(lox::as_variant<int64_t>(c_opt.value()), 10);
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
    EXPECT_EQ(lox::is_variant_v<lox::TypeError>(exec.value()), true);
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
    EXPECT_EQ(lox::is_variant_v<lox::TypeError>(exec.value()), true);
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
    const auto parse_result = parser.declaration();
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
    EXPECT_EQ(lox::is_variant_v<lox::UndefinedVariableError>(exec.value()), true);
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
    EXPECT_EQ(lox::is_variant_v<lox::TypeError>(exec.value()), true);
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
    EXPECT_EQ(lox::is_variant_v<lox::TypeError>(exec.value()), true);
  }

  {
    const std::string source = R"(
var a = (1+2;
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::UnmatchedParenError);
  }

  {
    const std::string source = R"(
var a;
a = (1 + 2;
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::UnmatchedParenError);
  }

  {
    const std::string source = R"(
var a
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
var 1 = 2 + 3;
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::MissingValidIdentifierDecl);
  }
}

TEST(Statement, if_statement_errors)
{
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if {
  b = 100;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::MissingIfConditon);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (b == 10 {
  b = 100;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::UnmatchedParenError);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (a == "123");
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::MissingIfBody);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (b == 10){
  b = 100
}
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
var a = "123";
var b = 10;
if (b == 10) {
  b = 100;
} else if(b == 100) {
  print b
}
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
var a = "123";
var b = 10;
if (b == 10) {
  b = 100;
} else if(b == 100) {
  print b;
} else
  print b;
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::UnmatchedBraceError);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (b == 10) {
  b = 100;
} else if(b == 100) {
  print b;
} else {
  print b
}
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
var a = 0;
var b = 0;
var c = 0;
while (a < 10 {
   c = c + 100;
   while(b < 10) {
      c = c + 10;
      b = b + 1;
   }
   a = a + 1;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::UnmatchedParenError);
  }
  {
    const std::string source = R"(
var a = 0;
var b = 0;
var c = 0;
while (a < 10) {
   c = c + 100;
   while(b < 10) {
      c = c + 10;
      b = b + 1;
   // missing }
   a = a + 1;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::UnmatchedBraceError);
  }
  {
    const std::string source = R"(
var a = 0;
var b = 0;
var c = 0;
while a < 10 {
   c = c + 100;
   while(b < 10) {
      c = c + 10;
      b = b + 1;
   }
   a = a + 1;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::MissingWhileConditon);
  }
  {
    const std::string source = R"(
var a = 0;
var b = 0;
var c = 0;
while (a < 10)
  print a;
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::MissingWhileBody);
  }
  {
    const std::string source = R"(
var a = 0;
for var b = 0; b < 10; b = b + 1 {
  a = a + 1;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::MissingForCondition);
  }
  {
    const std::string source = R"(
var a = 0;
for (var b = (1+2; b < 10; b = b + 1) {
  a = a + 1;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::UnmatchedParenError);
  }
  {
    const std::string source = R"(
var a = 0;
for ((1+2; b < 10; b = b + 1) {
  a = a + 1;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::UnmatchedParenError);
  }
  {
    const std::string source = R"(
var a = 0;
for (var b = 1+2; b < 10; b = (b + 1) {
  a = a + 1;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::UnmatchedParenError);
  }
  {
    const std::string source = R"(
var a = 0;
for (var b = 1+2; b < 10; b = b + 1)
  a = a + 1;
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::MissingForBody);
  }
  {
    const std::string source = R"(
var a = 0;
for (var b = 1+2; b < 10; b = b + 1) {
  a = a + 1
}
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
}

TEST(Statement, if_statement_runtime_errors)
{
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (c == 100){
  b = 100;
}
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

    const auto & err = exec.value();
    EXPECT_EQ(lox::is_variant_v<lox::UndefinedVariableError>(err), true);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (b == 10){
  b = 100 + "123";
}
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

    const auto & err = exec.value();
    EXPECT_EQ(lox::is_variant_v<lox::TypeError>(err), true);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (c == 100){
  b = 100;
}
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

    const auto & err = exec.value();
    EXPECT_EQ(lox::is_variant_v<lox::UndefinedVariableError>(err), true);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (b == 100){
  b = 100;
} else if (c == 100) {
// do something;
}
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

    const auto & err = exec.value();
    EXPECT_EQ(lox::is_variant_v<lox::UndefinedVariableError>(err), true);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (b == 100){
  b = 100;
} else if (b == 100) {
  // do something;
} else {
  print c;
}
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

    const auto & err = exec.value();
    EXPECT_EQ(lox::is_variant_v<lox::UndefinedVariableError>(err), true);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (c = "12345"; c != "12345"){
  b = 100;
} else if (var d = 12345; c != "12345") {
  b = d;
} else if (var e = 123456; d != 12345) {
  b = b + d + e;
} else {
  b = 54321;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::SyntaxError>(parse_result), true);
    const auto & err = lox::as_variant<lox::SyntaxError>(parse_result);
    EXPECT_EQ(err.kind, lox::SyntaxErrorKind::UnmatchedParenError);
  }
  {
    const std::string source = R"(
var a = "123";
var b = 10;
if (var c == "12345"; c != "12345"){
  b = 100;
} else if (var d = 12345; c != "12345") {
  b = d;
} else if (var e = 123456; d != 12345) {
  b = b + d + e;
} else {
  b = 54321;
}
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
var a = 123;
var b = 10;
var c = "before";
b = (a == 100 + "23") and (c == "be" + "fore");
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    const auto exec = interpreter.execute(program);
    EXPECT_EQ(exec.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<lox::TypeError>(exec.value()), true);
  }
  {
    const std::string source = R"(
var a = 123;
var b = 10;
var c = "before";
b = (a == 100 + 23) and (c == be + "fore");
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    const auto exec = interpreter.execute(program);
    EXPECT_EQ(exec.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<lox::UndefinedVariableError>(exec.value()), true);
  }
  {
    const std::string source = R"(
var a = 123;
var b = 10;
var c = "before";
b = (a == 100 + 23) and (a == 100 + "23");
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    const auto exec = interpreter.execute(program);
    EXPECT_EQ(exec.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<lox::TypeError>(exec.value()), true);
  }
  {
    const std::string source = R"(
var a = 0;
var b = 0;
var c = 0;
while (a < 10) {
   c = c + 100;
   while(d < 10) {
      c = c + 10;
      b = b + 1;
   }
   a = a + 1;
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    const auto exec = interpreter.execute(program);
    EXPECT_EQ(exec.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<lox::UndefinedVariableError>(exec.value()), true);
  }
  {
    const std::string source = R"(
var c = 0;
for (var a = 0 + "0"; a < 10; a = a + 1) {
   c = c + 10;
   for (var b = 0; b < 10; b = b + 1) {
      c = c + 20;
   }
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    const auto exec = interpreter.execute(program);
    EXPECT_EQ(exec.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<lox::TypeError>(exec.value()), true);
  }
  {
    const std::string source = R"(
var c = 0;
for (var a = 0; a < "10"; a = a + 1) {
   c = c + 10;
   for (var b = 0; b < 10; b = b + 1) {
      c = c + 20;
   }
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    const auto exec = interpreter.execute(program);
    EXPECT_EQ(exec.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<lox::TypeError>(exec.value()), true);
  }
  {
    const std::string source = R"(
var c = 0;
for (var a = 0; a < 10; a = a + "1") {
   c = c + 10;
   for (var b = 0; b < 10; b = b + 1) {
      c = c + 20;
   }
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    const auto exec = interpreter.execute(program);
    EXPECT_EQ(exec.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<lox::TypeError>(exec.value()), true);
  }
  {
    const std::string source = R"(
var c = 0;
for (var a = 0; a < 10; a = a + 1) {
   c = c + 10;
   for (var b = 0; b < 10; b = b + 1) {
      c = c + d;
   }
}
)";
    auto tokenizer = lox::Tokenizer(source);
    const auto result = tokenizer.take_tokens();
    EXPECT_EQ(lox::is_variant_v<lox::Tokens>(result), true);
    const auto & tokens = lox::as_variant<lox::Tokens>(result);

    auto parser = lox::Parser(tokens);
    const auto parse_result = parser.program();
    EXPECT_EQ(lox::is_variant_v<lox::Program>(parse_result), true);
    const auto & program = lox::as_variant<lox::Program>(parse_result);

    auto interpreter = lox::Interpreter{};
    const auto exec = interpreter.execute(program);
    EXPECT_EQ(exec.has_value(), true);
    EXPECT_EQ(lox::is_variant_v<lox::UndefinedVariableError>(exec.value()), true);
  }
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
