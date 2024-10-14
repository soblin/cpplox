#include <cpplox/interpreter.hpp>
#include <cpplox/parser.hpp>
#include <cpplox/tokenizer.hpp>

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

TEST(Statement, expr_statement)
{
  {
    const std::string source = R"(
(1 + 2) * ( 3 + 4);
)";

    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, _] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, _] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, _] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source1));
    const auto [program, tokens] = ParseProgramTest(source1);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
  a = a + 1;
  if (a > 10) {
    break;
    b = 10;
  } else {
    continue;
  }
}
)";
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

    auto interpreter = lox::Interpreter{};
    const auto exec = interpreter.execute(program);
    EXPECT_EQ(exec.has_value(), false);
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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

    auto interpreter = lox::Interpreter{};
    const auto exec = interpreter.execute(program);
    EXPECT_EQ(exec.has_value(), false);
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
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
  {
    const std::string source = R"(
var a = 0;
var b = 0;
var c = 0;
while (true) {
  a = a + 1;
  if (a > 10) {
    break;
  }
  while (true) {
    if (b > 10) {
      break;
    }
    b = b + 1;
  }
  c = c + 1;
}
)";
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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
  {
    const std::string source = R"(
var a = 0;
var b = 0;
var c = 0;
while (true) {
  a = a + 1;
  if (a > 10) {
    break;
  }
  while (true) {
    if (b > 10) {
      break;
    } else {
      b = b + 1;
      continue;
    }
    b = b + 10;
  }
  c = c + 1;
}
)";
    ASSERT_NO_FATAL_FAILURE(CheckParseProgramTest(source));
    const auto [program, tokens] = ParseProgramTest(source);

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

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
